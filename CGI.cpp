#include "CGI.hpp"


CGI::CGI() : _pid(-1), lastError(""), envp(NULL) {}
CGI::~CGI()
{
    if (envp)
    {
        for (size_t i = 0; envp[i] != NULL; ++i)
        {
            delete[] envp[i];
        }
        delete[] envp;
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

bool CGI::setUp(const std::string &script_filename, const std::string &request_body)
{
    std::ifstream file(script_filename.c_str());
    if(!file.good())
    {
        lastError = "Error : script file : " + script_filename + " does not exist";
        return false;
    }
    file.close();

    _script_filename = script_filename;

    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"]  = "POST";
    env["CONTENT_LENGTH"]  = std::to_string(request_body.length());
    env["CONTENT_TYPE"]    = "application/x-www-form-urlencoded";
    env["SCRIPT_NAME"]     = "/cgi-bin/" + script_filename;
    env["SERVER_NAME"]     = "localhost";
    env["SERVER_PORT"]     = "8080";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_SOFTWARE"] = "Webserv/1.0";
    env["PATH_INFO"]       = "";
    env["PATH_TRANSLATED"] = "";
    env["SCRIPT_FILENAME"] = script_filename;
    env["REDIRECT_STATUS"] = "200";
    env["DOCUMENT_ROOT"]   = ".";
    env["QUERY_STRING"]    = "";
    env["REQUEST_URI"]     = "/cgi-bin/" + script_filename;

    std::vector<std::string> env_list;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        env_list.push_back(it->first + "=" + it->second);
    }

    envp = new char*[env_list.size() + 1];
    for (size_t i = 0; i < env_list.size(); i++) {
        envp[i] = new char[env_list[i].size() + 1];
        std::strcpy(envp[i], env_list[i].c_str());
    }
    envp[env_list.size()] = NULL;

    return true;
}

pid_t CGI::execute(int &fdR, int &fdW)
{
    int pipe_stdin[2];
    int pipe_stdout[2];
    if(pipe(pipe_stdin) < 0)
    {
        lastError = "stdin pipe failed!";
        return -1;
    }
    if(pipe(pipe_stdout) < 0)
    {
        lastError = "stdout pipe faile";
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        return -1;
    }

    pid_t pid = fork();

    if(pid < 0)
    {
        lastError = "fork faild";
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        return -1;
    }
    if(pid == 0)
    {
        dup2(pipe_stdin[0], STDIN_FILENO);
        dup2(pipe_stdout[1], STDOUT_FILENO);
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);

        char *argv[] = {(char *)("/usr/bin/python3"), (char *)_script_filename.c_str(), NULL};
        execve(argv[0], argv, envp);
        _exit(1);
    }


    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    fdR = pipe_stdout[0];
    fdW = pipe_stdin[1];

    return pid;
}

static std::string trim(std::string &str)
{
    size_t trim_start = str.find_first_not_of(" \r\t\n");
    if(trim_start == std::string::npos)
    {
        return "";
    }
    size_t trim_end = str.find_last_not_of(" \r\t\n");
    return str.substr(trim_start, trim_end - trim_start + 1);
}

void convertToMap(const std::string &data)
{
    size_t line_start = 0;
    size_t fence = data.find("\r\n\r\n");
    size_t fence_len = 4;
    if(fence == std::string::npos)
    {
        fence = data.find("\n\n");
        fence_len = 2;
    }

    std::string header = (fence != std::string::npos) ? data.substr(0, fence) : data;
    std::string body = (fence != std::string::npos) ? data.substr(fence + fence_len) : "";

    std::map<std::string, std::string> converted;

    while(line_start < header.length())
    {
        size_t line_end = header.find('\n', line_start);
        if(line_end == std::string::npos)
        {
            line_end = header.length();
        }
        std::string line = header.substr(line_start, line_end - line_start);
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }   
        size_t delim_pos = line.find(":");
        if(delim_pos != std::string::npos)
        {
            std::string key = line.substr(0, delim_pos);
            std::string value = line.substr(delim_pos + 1);
            key = trim(key);
            value = trim(value);
            converted.insert(std::make_pair(key, value));
        }
        line_start = line_end + 1;
    }

    std::map<std::string, std::string>::iterator status_it = converted.find("Status");
    if(status_it != converted.end())
    {
        std::cout << "HTTP/1.1 " << status_it->second << "\n";
        converted.erase(status_it);
    }
    else {
        std::cout << "HTTP/1.1 200 OK\n";
    }

    for(std::map<std::string, std::string>::iterator it = converted.begin(); it != converted.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "\n";
    std::cout << body;
}


