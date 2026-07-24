#include "CGI.hpp"
#include <vector>
#include <cstring>

// Constructor: initialises _pid to -1 (no child process yet), lastError to an empty string, and envp to NULL (no environment array allocated)
CGI::CGI() : _pid(-1), lastError(""), envp(NULL) {}
// Destructor: frees all memory allocated for the environment variable array
CGI::~CGI()
{
    // Only attempt cleanup if envp was previously allocated
    if (envp)
    {
        // Iterate over the NULL-terminated array and delete each individual C-string
        for (size_t i = 0; envp[i] != NULL; ++i)
        {
            delete[] envp[i];
        }
        // Delete the array of pointers itself
        delete[] envp;
        // Set the pointer to NULL to prevent dangling-pointer issues
        envp = NULL;
    }
}
std::string CGI::getError() const
{
    return lastError;
}
pid_t CGI::getPid() const
{
    return _pid;
}

// Prepares the CGI execution environment: verifies the script exists, stores metadata, and builds the envp array
bool CGI::setUp(const std::string &script_filename, const std::string &request_body, const std::string &interpreter)
{
    // Store the interpreter path for later use in execve()
    _interpreter = interpreter;
    // Attempt to open the script file to verify it exists and is accessible
    std::ifstream file(script_filename.c_str());
    // If the file stream indicates an error (file doesn't exist, no permissions, etc.)
    if(!file.good())
    {
        lastError = "Error : script file : " + script_filename + " does not exist";
        return false;
    }
    // Close the file since we only needed to verify its existence
    file.close();

    // Store the script filename for later use in argv for execve()
    _script_filename = script_filename;

    // Build a map of CGI environment variables that the script will receive
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"]  = "POST";                     // The HTTP method used for the request
    env["CONTENT_LENGTH"]  = std::to_string(request_body.length()); // Length of the POST body in bytes
    env["CONTENT_TYPE"]    = "application/x-www-form-urlencoded";   // MIME type of the POST body
    env["SCRIPT_NAME"]     = "/cgi-bin/" + script_filename;         // Virtual path to the script
    env["SERVER_NAME"]     = "localhost";                 // The server's hostname
    env["SERVER_PORT"]     = "8080";                      // The port on which the server is listening
    env["SERVER_PROTOCOL"] = "HTTP/1.1";                  // The HTTP protocol version used
    env["GATEWAY_INTERFACE"] = "CGI/1.1";                 // The CGI specification version
    env["SERVER_SOFTWARE"] = "Webserv/1.0";               // Name/version of the server software
    env["PATH_INFO"]       = "";                          // Extra path info (empty in this minimal example)
    env["PATH_TRANSLATED"] = "";                          // Translated version of PATH_INFO (empty by default)
    env["SCRIPT_FILENAME"] = script_filename;             // Actual filesystem path to the script
    env["REDIRECT_STATUS"] = "200";                       // Required by some CGI implementations (PHP-FPM, etc.)
    env["DOCUMENT_ROOT"]   = ".";                         // The server's document root directory
    env["QUERY_STRING"]    = "";                          // The raw query string from the URL (empty for POST)
    env["REQUEST_URI"]     = "/cgi-bin/" + script_filename; // The full URI path of the request

    // Temporary vector to hold each env entry as a "KEY=VALUE" string before converting to C-strings
    std::vector<std::string> env_list;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        env_list.push_back(it->first + "=" + it->second);
    }

    // Allocate a NULL-terminated array of char* pointers (one more slot than entries for the NULL terminator)
    envp = new char*[env_list.size() + 1];
    // Convert each std::string env entry into a dynamically allocated C-string
    for (size_t i = 0; i < env_list.size(); i++) {
        envp[i] = new char[env_list[i].size() + 1];  // +1 for the null terminator
        std::strcpy(envp[i], env_list[i].c_str());   // Copy the string data into the allocated buffer
    }
    // NULL-terminate the array as required by execve()
    envp[env_list.size()] = NULL;

    return true;
}

// Forks a child process, sets up bidirectional Unix socket communication, and exec()s the CGI script
pid_t CGI::execute(int &fdR, int &fdW)
{
    // pipe_stdin holds the socket fds used for writing TO the CGI's stdin (index 1) and reading from it (index 0)
    int pipe_stdin[2];
    // pipe_stdout holds the socket fds used for reading FROM the CGI's stdout (index 0) and writing to it (index 1)
    int pipe_stdout[2];
    // Create a pair of connected Unix-domain stream sockets for the stdin channel
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_stdin) < 0)
    {
        lastError = "stdin socketpair failed!";
        return -1;
    }
    // Create a pair of connected Unix-domain stream sockets for the stdout channel
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_stdout) < 0)
    {
        lastError = "stdout socketpair failed!";
        // Clean up the previously created stdin sockets before returning the error
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        return -1;
    }

    // Set the close-on-exec flag on all four socket fds so they are automatically closed in the child after exec()
    fcntl(pipe_stdin[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_stdin[1], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_stdout[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_stdout[1], F_SETFD, FD_CLOEXEC);

    // Fork the current process: creates a new child process that is a copy of the parent
    pid_t pid = fork();

    // If fork() returns a negative value, the fork failed
    if(pid < 0)
    {
        lastError = "fork failed";
        // Close all four socket descriptors to avoid resource leaks
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        return -1;
    }
    // If fork() returned 0, we are in the child process
    if(pid == 0)
    {
        // Duplicate the read-end of the stdin socket onto file descriptor 0 (STDIN_FILENO)
        dup2(pipe_stdin[0], STDIN_FILENO);
        // Duplicate the write-end of the stdout socket onto file descriptor 1 (STDOUT_FILENO)
        dup2(pipe_stdout[1], STDOUT_FILENO);
        // Close the original socket descriptors (no longer needed after dup2)
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);

        // If an interpreter was specified (e.g., "/usr/bin/python3"), use it to run the script
        if(!_interpreter.empty())
        {
            // Build argv: [interpreter_path, script_filename, NULL]
            char *argv[] = {(char *)_interpreter.c_str(), (char *)_script_filename.c_str(), NULL};
            // Replace the child process image with the interpreter running the script, passing the envp array
            execve(argv[0], argv, envp);
        }
        else
        {
            // If no interpreter, assume the script is executable on its own (has a shebang line)
            char *argv[] = {(char *)_script_filename.c_str(), NULL};
            execve(argv[0], argv, envp);
        }
        // If execve() returns, it means it failed; terminate the child process immediately with status 1
        _exit(1);
    }

    // Parent process continues here:

    // Close the child's side of the sockets — the parent does not need the read-end of stdin or the write-end of stdout
    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    // fdR is the read-end of the stdout socket — the parent reads CGI output from this fd
    fdR = pipe_stdout[0];
    // fdW is the write-end of the stdin socket — the parent writes request body data to this fd
    fdW = pipe_stdin[1];

    // Set the read fd to non-blocking mode so read() returns immediately with EAGAIN if no data is available
    fcntl(fdR, F_SETFL, O_NONBLOCK);
    // Set the write fd to non-blocking mode so write() does not block if the kernel buffer is full
    fcntl(fdW, F_SETFL, O_NONBLOCK);

    // Store the child PID so the caller can waitpid() for it later
    _pid = pid;
    return _pid;
}

// Helper function: removes leading and trailing whitespace characters (space, carriage return, tab, newline) from a string
static std::string trim(std::string &str)
{
    // Find the first character that is NOT a whitespace character
    size_t trim_start = str.find_first_not_of(" \r\t\n");
    // If no non-whitespace character exists, the string is entirely whitespace
    if(trim_start == std::string::npos)
    {
        return "";
    }
    // Find the last character that is NOT a whitespace character
    size_t trim_end = str.find_last_not_of(" \r\t\n");
    // Return the substring from the first to the last non-whitespace character (inclusive)
    return str.substr(trim_start, trim_end - trim_start + 1);
}

// Parses the raw CGI response (headers separated from body by \r\n\r\n or \n\n) and prints formatted HTTP output to std::cout
void convertToMap(const std::string &data)
{
    // Index used to track the current position as we iterate through header lines
    size_t line_start = 0;
    // Search for the standard HTTP header separator: a blank line consisting of \r\n\r\n (CRLF CRLF)
    size_t fence = data.find("\r\n\r\n");
    // The length of the separator: 4 characters for \r\n\r\n
    size_t fence_len = 4;
    // If the CRLF CRLF separator was not found, try the simpler \n\n separator (LF LF)
    if(fence == std::string::npos)
    {
        fence = data.find("\n\n");
        fence_len = 2;
    }

    // Extract the header portion: everything before the blank-line separator, or the whole string if no separator exists
    std::string header = (fence != std::string::npos) ? data.substr(0, fence) : data;
    // Extract the body portion: everything after the separator, or an empty string if no separator exists
    std::string body = (fence != std::string::npos) ? data.substr(fence + fence_len) : "";

    // Map to store parsed header key-value pairs (keys are header field names, values are the corresponding values)
    std::map<std::string, std::string> converted;

    // Loop through each line of the header section
    while(line_start < header.length())
    {
        // Find the end of the current line (the next newline character)
        size_t line_end = header.find('\n', line_start);
        if(line_end == std::string::npos)
        {
            line_end = header.length();
        }
        // Extract the current line from line_start to line_end
        std::string line = header.substr(line_start, line_end - line_start);
        // Remove a trailing carriage return (\r) if present (to handle CRLF line endings)
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }
        // Find the colon that separates the header key from its value
        size_t delim_pos = line.find(":");
        if(delim_pos != std::string::npos)
        {
            // Extract the key (everything before the colon)
            std::string key = line.substr(0, delim_pos);
            // Extract the value (everything after the colon)
            std::string value = line.substr(delim_pos + 1);
            // Trim whitespace from both key and value
            key = trim(key);
            value = trim(value);
            // Insert the key-value pair into the map
            converted.insert(std::make_pair(key, value));
        }
        // Move to the start of the next line
        line_start = line_end + 1;
    }

    // Look for a "Status" header in the parsed map (CGI scripts may return a custom status like "404 Not Found")
    std::map<std::string, std::string>::iterator status_it = converted.find("Status");
    if(status_it != converted.end())
    {
        // If a Status header exists, print it as the HTTP status line
        std::cout << "HTTP/1.1 " << status_it->second << "\n";
        // Remove the Status entry from the map so it isn't printed again as a regular header
        converted.erase(status_it);
    }
    else {
        // If no Status header, default to HTTP/1.1 200 OK
        std::cout << "HTTP/1.1 200 OK\n";
    }

    // Print all remaining parsed headers in "Key: Value" format
    for(std::map<std::string, std::string>::iterator it = converted.begin(); it != converted.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    // Print a blank line to separate headers from the body (as required by the HTTP protocol)
    std::cout << "\n";
    // Print the response body
    std::cout << body;
}


