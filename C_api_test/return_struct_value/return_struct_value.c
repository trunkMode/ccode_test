#include <stdio.h>

struct A
{
    int aa[100];
    int bb[100];
    int cc[100];
    long long a;
    long long b;
    long long c;
};

struct A add(int x, int y)
{
#if 1
    struct A t;
    
    t.a = 100;
    t.b = 200;
    t.c = 300;
    return t;
#else
    struct A t;
    int* p = &x;
    p--;
    printf("address of return struct: %08X\n", *p);
    t.a = x * y;
    return t;
#endif
}

int main(int argc, char* argv[])
{
#if 0
    struct A t = add(3,4);
    struct A *p = &t;
    printf("t.a = %d\n", t.a);
    printf("t.b = %d\n", t.b);
    printf("t.c = %d\n", t.c);
    
    printf("p->a = %d\n", p->a);
    printf("p->b = %d\n", p->b);
    printf("p->c = %d\n", p->c);
#else
    struct A t = add(3, 4);
    struct A *p1 = &t;

    printf("address of t in main: %p\n", &t);
    return 0;
#endif


}
