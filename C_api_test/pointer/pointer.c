#include <unistd.h>


int main()
{
    char p[100] = {0};
    char *q = "hello world\n";

    char *dst = p, *src = q;

//    while ( ( *(dst++) = *(src++) ) );
    while (( *dst++ = *src++ ));
    printf("dst = %s\n", p);
}
