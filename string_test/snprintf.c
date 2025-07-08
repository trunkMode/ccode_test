#include <unistd.h>
#include <stdio.h>


int main()
{
    const char *str = "test test";
    char a[5] = {0};
    int ret = snprintf(a, sizeof(a), "%s", str);
    printf("a = %s, ret = %d\n", a, ret);
    return ret;
}
