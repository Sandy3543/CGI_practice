#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

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
    int num = std::stoi(html_body.substr(start_pos, end_pos - start_pos));
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
        std::cerr << " pipe failed\n";
        return 1;
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        return 1;
    }
    else if(pid == 0)
    {
        //std::cout << "child process start\n";
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        char *args[] = {(char *)"python3", (char *)"mult.py", (char *)"3", (char *)"2", NULL};
        execv("/usr/bin/python3", args);
        std::cerr << "child execv failed\n";
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