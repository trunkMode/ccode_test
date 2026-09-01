typedef struct {
    char a;
    int b;
    char c;
} TEST __attribute__((packed));

int main()
{
    char b;
    TEST t;
    char  a[10] = {1, 2, 3, 4, 5, 6};
    int *ptr = a;
    printf("0x%x, a = %p, b = %p\n", *ptr, a, &b);

    printf("&t.b = %p, sizeof(t) = %d\n", &t.b, sizeof(t));

    ptr = b;
    printf("b = %p, 0x%x, a = %p\n", &b, *ptr, &b);
    
    return 0;
}
