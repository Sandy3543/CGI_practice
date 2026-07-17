#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();
    if(pid < 0)
    {
        std::cout << "fork faild";
    }
    else if(pid == 0)
    {
        std::cout << "childe process start\n";
        char *args[] ={(char *)"/bin/ls", (char *)"-l", NULL};
        execv("/bin/ls", args);

        std::cerr << "if this line appear then execv failed\n";
    }
    else{
        std::cout << "parent waiting for child to finish\n";
        wait(NULL);
        std::cout << "child finished";
    }
}