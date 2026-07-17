#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

int main()
{
    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        std::cerr << "pipe failed\n";
        return 1;
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        std::cerr << "fork failed \n";
        return 1;
    }
    else if(pid == 0)
    {
        dup2(pipefd[0], STDIN_FILENO);
        std::cout << "child pro started\n";
        close(pipefd[1]);
        close(pipefd[0]);
        char *greparg[] = {(char *)"/usr/bin/grep", (char *)"pipe", NULL};
        execv("/usr/bin/grep", greparg);   
        std::cerr << "child exec failed\n";
        return 1;
    }
    else{
        std::cout << "parent pro started\n";
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        char *arg[] = {(char *)"/bin/ls", (char *)"-l", NULL};
        execv("/bin/ls", arg);
        std::cerr << "parent exec failed\n";
        return 1;
    }
}