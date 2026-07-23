#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <map>


class CGI
{
    private:
        pid_t _pid;
        std::string lastError;
        std::string _script_filename;
        char **envp;

    public:
        CGI();
        ~CGI();
        bool setUp(const std::string &script_filename, const std::string &request_body);
        pid_t execute(int &fdR, int &fdW);

        pid_t getPid() const;
        std::string getError() const;
};

void convertToMap(const std::string &data);



#endif 