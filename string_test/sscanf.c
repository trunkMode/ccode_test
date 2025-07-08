#include <unistd.h>

#define UB_KEY "uboot_version"
#define UB_FMT "%*[^"UB_KEY"]"UB_KEY"%s"
int main()
{
    const char *str = "console=ttyMSM0,115200n8 snwl_irv=0x650000 prodcode=0x14a0 uboot_version=8.7.0.0-0o";
    char buf[128] = {0};
    int m = 0;

    sscanf("11 12 13", "%d ", &m);
    printf("m = %d\n", m);

    /* get desired input */
    m = sscanf(str, "%*[^ub]uboot_version=%s", buf);
    printf("fmt = %s, buf = %s, m = %d\n", UB_FMT, buf, m);
}
