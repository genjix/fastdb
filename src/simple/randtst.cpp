#include <stdlib.h>
#include <iostream>

int main()
{
    srand(110);
    for (size_t i = 0; i < 20; ++i)
        std::cout << rand() << std::endl;
    return 0;
}

