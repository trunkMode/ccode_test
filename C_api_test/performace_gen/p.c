#include <unistd.h>

int main()
{
    unsigned long long i = 0;
    while (1) {
        i++;
        if (i >  1000000000)
            break;
    }
    return 0;
}
