#include <unistd.h>

#if 0
NAME
       bsearch - binary search of a sorted array

       SYNOPSIS
              #include <stdlib.h>

              void *bsearch(const void *key, const void *base,
                                           size_t nmemb, size_t size,
                                           int (*compar)(const void *, const void *));

#endif
int bsearch_my(int *a, int len, int search)
{
    int s, m, e;

    s = 0;
    e = len - 1;

    printf("%s %d\n", __func__, __LINE__);
    while (s <= e) {
        m = (s + e) >> 1;

        printf("s: %d e:%d m: %d\n", s, e, m);
        if (search < a[m]) {
            e = m - 1;
        } else if (search == a[m]) {
            printf("find in index %d\n", m);
            break;
        } else if (search > a[m]) {
            s = m + 1;
        }
    }
        printf("s: %d e:%d m: %d\n", s, e, m);
}


int main()
{
    int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    bsearch_my(a, 9, 5);
    bsearch_my(a, 9, 115);
    return 0;
}
