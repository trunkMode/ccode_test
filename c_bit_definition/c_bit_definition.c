#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#if 1
struct test{
    unsigned int a:1;
    unsigned int b:6;
    unsigned int c:4;
    unsigned int d:5;
    unsigned int e;
};
#endif
struct sta_info_node {
    unsigned char   valid_host:1,
                    valid_os:1,
                    valid_ssid:1,
                    valid_radio:1;
    const uint8_t *pMac;
    union {
        char hostname[32];
        char os[32];
        struct {
            char ssid[32];
        };
    }u;
};


void union_test()
{
    struct sta_info_node sta_info;
    snprintf(sta_info.u.hostname, 32, "%s", "hello");
    printf("sta_info.u.ssid = %s\n", sta_info.u.ssid);

}
int main()
{
    struct test test;
    test.a = 1;
    test.b = 0xf;
    test.c = 0xf;
    test.d=0x1f;
    test.e =0xffff;
    printf("a = %x, b = %x, c = %x, d = %x, e = %x\n", test.a, test.b, test.c, test.d, test.e);

    /*union_test*/
    union_test();

}
