#include <unistd.h>

int main()
{
    int ret = strncmp("", "11", 0);
    printf("ret = %d\n", ret);
    return 0;
}

