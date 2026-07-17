#include <iostream>
#include<vector>
#include<map>

int main()
{
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = "GET";
    env["QUERY_STRING"] = "a=3&b=5";
    std::vector<std::string> env_list;
    for(std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++)
    {
        std::string line = it->first + "=" + it->second;
        env_list.push_back(line);
    }
}