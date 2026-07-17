#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

int add_ten(int num)
{
    std::cin >> num;
    num = num + 10;
    return num;
}

int main()
{
    int n = 0;
    int num = add_ten(n);
    std::cout << num;
}
