#include <unistd.h>
#include <stdio.h>
#include <stdint.h>


int main()
{
    int ret = 0;
    uint8_t *buf = NULL;
    FILE *fp = fopen("test.bin", "w");

    do {
        if (!fp) {
            printf("Failed to open file\n");
            break;
        }

        if (!(buf = malloc(1024 * 1024))) {
            break;
        }

        ret = fwrite(buf, 1, 1024 *1024, fp);
        printf("write %d bytes (%d)\n", ret, 1024 *1024);
    } while (0);

    if (fp)
        fclose(fp);
    return 0;
}
