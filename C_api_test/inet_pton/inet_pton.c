#include <arpa/inet.h>

#if 0
       const char *inet_ntop(int af, const void *src,
                                            char *dst, socklen_t size);
#endif
//int inet_pton(int af, const char *src, void *dst);
int main()
{
    int ret;
    char buf[128] = {0};
    struct in_addr inaddr;
    char *p = malloc(100), *q;

    memset(&inaddr, 0x00, sizeof(inaddr));

    ret = inet_pton(AF_INET, "192.168.1.1", &inaddr);
    printf("ret = %d\n", ret);

    inet_ntop(AF_INET, &inaddr, buf, sizeof(buf));
    printf("buf = %s\n", buf);

    buf[129] = '\0';

    free(p);
    q = malloc(100);

    *p = 0x11;
    p[10] = 0x99;


    return 0;
}
