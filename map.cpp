#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <map>

int main()
{
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = "GET";
    env["QUERY_STRING"] = "a=5&b=6";

    for(std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++)
    {
        std::string ful_env_var = it->first + "=" + it->second;
        std::cout << "Built env variable : " << ful_env_var << std::endl;
    }

}