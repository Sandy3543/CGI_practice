#include <iostream>
#include <unistd.h>

int main()
{

    pid_t pid = fork();
    if(pid < 0)
    {
        std::cout << "fork faild" << std::endl;
    }
    else if(pid == 0)
    {
        std::cout << "child process start" << std::endl;
        for(int i = 1; i <= 3; i++)
        {
            std::cout << "child step : " << i << std::endl;
            std::cout << "child pid = " << getpid() << std::endl; 
        }
    }
    else{
        std::cout << "parent process start" << std::endl;
        for(int i = 1; i <= 3; i++)
        {
            std::cout << "parent step : " << i << std::endl;
            std::cout <<"parent pid : " << getpid() << std::endl;
            sleep(1);
        }
        wait(NULL);
        std::cout << "[PARENT] Child has finished. Parent exiting now.\n";
    }

}