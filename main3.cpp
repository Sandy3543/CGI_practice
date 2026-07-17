#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<map>
#include<vector>
#include<cstring>
#include<string>
#include<cstdlib>

int add_ten()
{
    std::string header;
    std::string empty_line;
    std::string html_body;
    std::getline(std::cin, header); 
    std::getline(std::cin, empty_line);
    std::getline(std::cin, html_body);

    size_t start_pos = html_body.find("Result: ") + 8;
    size_t end_pos = html_body.find("</h1>");
    int num = std::atoi(html_body.substr(start_pos, end_pos - start_pos).c_str());
    int final_result = num + 10;
    std::cout << "content-type: text/html\n\n";
    std::cout << "<html><body><h2>Server Final Result (+10): " << final_result << "</h2></body></html>\n";
    return final_result;
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
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        int num = add_ten();
        //std::cout << num;
    }
    wait(NULL);
    std::cout << "\nchild finished\n";
}