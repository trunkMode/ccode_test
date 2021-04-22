#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
//#define CLIENT_CERT "client_secp384r1.p12"
#define CLIENT_CERT "client.p12"
#define BUF_LEN     (500*1024)
int main()
{
    int ret = 0;
    FILE *fp = NULL;
    uint8_t *buf = NULL;

    do {
        fp = fopen(CLIENT_CERT, "r");
        if (!fp) {
            printf("Failed to open file\n");
            break;
        }

        buf = malloc(BUF_LEN);
        if (!buf) {
            printf("Failed to allocate buf\n");
            break;
        }

        ret = fread(buf, 1, BUF_LEN, fp);
        printf("ret = %d\n", ret);
        printf("b64 = %s\n", b64encode(buf, ret));

    } while(0);

    if (fp)
        fclose(fp);

    free(buf);
    return 0;
}

