#include <unistd.h>

int main()
{
    int i;
    int a[10][2] = {{1,4},{5,8},{9,12},{13,15}};
    int (*p)[2] = a;

    for (i = 0; i < 10; i++) {
        printf("p[%d][%d] = %d\n", i, 0, p[i][0]);
    }
    return 0;
}
