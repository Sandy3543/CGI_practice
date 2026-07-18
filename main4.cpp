#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<map>
#include<vector>
#include<cstring>
#include<string>
#include<cstdlib>
#include<sstream>

void convertToMap(const char *buffer)
{
    std::map<std::string, std::string> converted;
    std::string data(buffer);
    size_t line_start = 0;
    size_t fence = data.find("\r\n\r\n");
    size_t fence_len = 4;
    if(fence == std::string::npos)
    {
        fence = data.find("\n\n");
        fence_len = 2;
    }
    std::string header = data.substr(line_start, fence);
    std::string body = data.substr(fence + fence_len);
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

            converted.insert(std::make_pair(key, value));
        }
        line_start = line_end + 1;
    }
    for(std::map<std::string, std::string>::iterator it = converted.begin(); it != converted.end(); it++)
    {
        std::cout<< it->first << ":" << it->second << std::endl;
    }
    std::cout << "\n\n";
    std::cout << body;
}

int add_ten(int num)
{
    return num + 10;
}
int main()
{
    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        std::cerr << "pipe failed";
        return 1;
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        std::cerr << "fork failed";
        return 1;
    }
    else if(pid == 0)
    {
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        std::map<std::string, std::string> env;
        env["REQUEST_METHOD"] = "GET";
        env["QUERY_STRING"] = "a=2&b=8";

        std::vector<std::string> env_list;
        for(std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++)
        {
            std::string line = it->first + "=" + it->second;
            env_list.push_back(line);
        }

        char **envp = new char*[env_list.size() + 1];
        for(size_t i = 0; i < env_list.size(); i++)
        {
            envp[i] = new char[env_list[i].size() + 1];
            std::strcpy(envp[i], env_list[i].c_str());
        }
        envp[env_list.size()] = NULL;

        char *args[] = {(char *)"/usr/bin/python3", (char *)"mult2.py", NULL};
        execve("/usr/bin/python3", args, envp);
        std::cerr << "execve failed";

        for(size_t i = 0; i < env_list.size(); i++)
        {
            delete[] envp[i];
        }
        delete[] envp;
        
        return 1;
    }
    else
    {
        close(pipefd[1]);
        int bytes_read;
        char buffer[1024];
        bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if(bytes_read < 0)
        {
            std::cerr << "read failed";
            return 1;
        }
        buffer[bytes_read] = '\0';
        //std::cout << buffer;
        close(pipefd[0]);
        //std::cout << num;
        convertToMap(buffer);
    }
    wait(NULL);
    //std::cout << "\nchild finished\n";
}