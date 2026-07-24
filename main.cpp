#include "CGI.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <ctime>

int main(int argc, char** argv) {
    std::string script = (argc > 1) ? argv[1] : "mult2.py";
    std::string interpreter = "/usr/bin/python3";
    std::string request_body = "a=5&b=10";

    CGI cgi;
    if (!cgi.setUp(script, request_body, interpreter)) return 1;

    int fdR = -1, fdW = -1;
    if (cgi.execute(fdR, fdW) < 0) return 1;

    // --- STEP 1: Send the body to the child, then close write end ---
    write(fdW, request_body.c_str(), request_body.length());
    close(fdW); // Signal EOF to child right away

    // --- STEP 2: Simple select() read loop ---
    std::string response = "";
    char buffer[1024];
    pid_t child_pid = cgi.getPid();
    time_t start_time = time(NULL);


    while (true) {
        int status = 0;
        pid_t result = waitpid(child_pid, &status, WNOHANG);
        if(result == child_pid) {
            std::cout << "[Child process exited]\n";
            break;
        }
        else if(result == 0) {
            if(time(NULL) - start_time >= 5) {
                std::cout << "[Timeout reached, killing child]\n";
                kill(child_pid, SIGKILL);
                waitpid(child_pid, NULL, 0);
                break;
            }
        }
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fdR, &read_fds); // Tell select: "watch fdR for incoming data"

        struct timeval timeout;
        timeout.tv_sec = 1; // 1 second timeout
        timeout.tv_usec = 0;

        // Wait until fdR has data ready (timeout = NULL means wait indefinitely)
        if (select(fdR + 1, &read_fds, NULL, NULL, &timeout) <= 0)
        {
            if (FD_ISSET(fdR, &read_fds)) {
                ssize_t bytes = read(fdR, buffer, sizeof(buffer) - 1);
                if (bytes <= 0) 
                {
                    break; // 0 means child closed connection (EOF), < 0 means error
                }
                buffer[bytes] = '\0';
                response.append(buffer);
                std::cout << "[Received chunk!]\n";
            }
        }
    }

    close(fdR);

    // --- STEP 3: Print result & wait for child ---
    std::cout << "\n--- Final CGI Output ---\n" << std::endl;
    convertToMap(response);

    int status;
    waitpid(cgi.getPid(), &status, 0);

    return 0;
}