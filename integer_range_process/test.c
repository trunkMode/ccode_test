typedef struct {
    int a;
    int b;
}info_a_t;

typedef struct {
    char c;
    int d;
}int_b_t;

int main()
{
    int i;
    info_a_t a;
    info_a_t b;

    a.a = 88;
    a.b = 77;

    b = a;
    printf("b.a = %d\n", b.a);
    for (i = 1; i; i--) {
        printf("i = %d\n",i);
    }
}
