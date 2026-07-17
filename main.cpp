#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

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

    pid_t pid2 = fork();
    if(pid2 < 0)
    {
        std::cerr << "fork2 failed\n";
        return 1;
    }
    else if(pid2 == 0)
    {
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    char *arg[] = {(char *)"./add_ten", NULL};
    execv("./add_ten", arg);
    std::cerr << "child2 execv failed\n";
    return 1;
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
    std::cout << "\nboth childs finished\n";
}