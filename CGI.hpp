#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <map>
#include <sys/socket.h>
#include <fcntl.h>
#include <cerrno>

// Forward declarations of the CGI class, which encapsulates the execution of a CGI script
class CGI
{
    private:
        // Stores the process ID of the forked child process running the CGI script
        pid_t _pid;
        // Stores a human-readable error message when an operation fails
        std::string lastError;
        // The filesystem path to the CGI script that should be executed
        std::string _script_filename;
        // The interpreter (e.g., "/usr/bin/python3") used to run the script; empty if the script is executable on its own
        std::string _interpreter;
        // A NULL-terminated array of C-strings representing the environment variables passed to the CGI process

        char **envp;

    public:
        // Constructor: initialises _pid to -1, lastError to an empty string, and envp to NULL
        CGI();
        // Destructor: frees all dynamically allocated memory in the envp array
        ~CGI();
        // Prepares the CGI environment: validates the script file exists, stores interpreter/filename, and builds the envp array
        bool setUp(const std::string &script_filename, const std::string &request_body, const std::string &interpreter);
        // Forks a child process, sets up socketpair communication, exec()s the CGI script, and returns file descriptors for reading/writing
        pid_t execute(int &fdR, int &fdW);

        // Returns the PID of the child process (used later for waitpid())
        pid_t getPid() const;
        // Returns the last error message (e.g., "fork failed", "stdin socketpair failed!")
        std::string getError() const;
};

// Free function that parses a raw CGI response (headers + body) and prints formatted HTTP output to std::cout
void convertToMap(const std::string &data);

#endif 