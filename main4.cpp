#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<map>
#include<vector>
#include<cstring>
#include<string>
#include<cstdlib>

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
        std::cout << "Received from child: " << buffer;
        close(pipefd[0]);
        //std::cout << num;
    }
    wait(NULL);
    std::cout << "\nchild finished\n";
}