#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct {
    uint32_t cidr;
    uint32_t prefix;
} cidr_t;

int range2cidr(cidr_t *cidr, size_t cidr_max, uint32_t start, uint32_t end, int max_prefix)
{
    uint32_t mask_map[32] = {
        0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
        0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00, 0xfffffe00,
        0xfffff800, 0xfffff000, 0xffffe000, 0xffff8000, 0xffff0000,
        0xfffe0000, 0xfff80000, 0xfff00000, 0xffe00000, 0xff800000,
        0xff000000, 0xfe000000, 0xf8000000, 0xf0000000, 0xe0000000,
        0x80000000, 0x00000000,
    };
    int i = 0, j, idx = 0;
    uint32_t s = start, e, value = start;

    memset(cidr, 0x00, sizeof(cidr_t) * cidr_max);
    s = start;
    while (s <= end) {
        for (i = 0; i < max_prefix; i++) {
            if (s & (1 << i)) {
                break;
            }
        }

        if (0 == i) {
            cidr[idx].cidr = s;
            cidr[idx].prefix = max_prefix;
            idx++;
            s++;
        } else {
            for (j = i; j >= 0; j--) {
                value = s + pow(2,j) - 1;
                if (value >= s && value <= end) {
                    cidr[idx].cidr = s & mask_map[j];
                    cidr[idx].prefix = max_prefix - j;
                    idx++;
                    s = value + 1;
                    break;
                }
            }
        }
    }


    printf("============= \n");
    for (i = 0; i < idx; i++) {
        printf("%d/%d, \n", cidr[i].cidr, cidr[i].prefix);
    }
    return 0;
}

int main()
{
    cidr_t cidr[32];

    range2cidr(cidr, 32, 10, 10, 16);
    range2cidr(cidr, 32, 11, 11, 16);
    range2cidr(cidr, 32, 500, 200, 32);
    return 0;
}
