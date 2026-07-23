#include "CGI.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

int main(int ac, char *av[])
{
    std::string script = (ac > 1)? av[1] : "script.py";
    std::string requestBody = "name=Sandy&age=25";

    CGI cgi;
    if(!cgi.setUp(script, requestBody, "/usr/bin/python3"))
    {
        std::cerr << cgi.getError() << std::endl;
        return 1;
    }
    int fdR = -1;
    int fdW = -1;
    if(cgi.execute(fdR, fdW) < 0)
    {
        std::cerr << cgi.getError() << std::endl;
        return 1;
    }
    write(fdW, requestBody.c_str(), requestBody.length());
    close(fdW);

    char buffer[1024];
    ssize_t read_bytes = read(fdR, buffer, sizeof(buffer) - 1);
    if(read_bytes > 0)
    {
        buffer[read_bytes] = '\0';
        std::cout << " ----- CGI OUTPUT --------" << std::endl;
        convertToMap(buffer);
    }
    waitpid(cgi.getPid(), NULL, 0);
}
