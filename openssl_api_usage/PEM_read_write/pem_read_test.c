#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <openssl/pem.h>

#define CCERT_MARK          "CERTIFICATE"
#define CCERT_PKEY_MARK     "PRIVATE KEY"
#define PEM_FILE            "ct.pem"
#define CCERT_PKEY_FILE     "ccert.key"
#define CCERT_FILE          "ccert.cert"

int spilt_spem(const char *path)
{
    int ret;
    FILE *fp_in = NULL, *fp_key = NULL, *fp_cert = NULL;
    char *name = NULL, *header = NULL;
    uint8_t *data = NULL;
    long len;

    do {
        if (!(fp_in = fopen(path, "r"))) {
            printf("Failed to open %s \n", path);
            break;
        }

        if (!(fp_key = fopen(CCERT_PKEY_FILE, "w"))) {
            printf("Failed to open %s\n", CCERT_PKEY_FILE);
            break;
        }

        if (!(fp_cert = fopen(CCERT_FILE, "w"))) {
            printf("Failed to open %s\n", CCERT_FILE);
            break;
        }
        do {
            len = 0;
            name = header = data = NULL;
            if (!(ret = PEM_read(fp_in, &name, &header, &data, &len))) {
                printf("PEM_read fail (%d)\n", ret);
                break;
            }

            if (!name || !data || !len) {
                free(name);
                free(data);
                free(header);
                continue;
            }

            printf("name = %s, header = %s, len = %d\n", name, header, len);
            if (!strcmp(name, CCERT_MARK)) {
                for (int i = 0; i < len; i++) {
                    printf("%02x", data[i]);
                }
                if (!PEM_write(fp_cert, name, "", data, len)) {
                    printf("Failed to write cert to %s\n", CCERT_FILE);
                }
            } else if (!strcmp(name, CCERT_PKEY_MARK)) {
                if (!PEM_write(fp_key, name, "", data, len)) {
                    printf("Failed to write cert to %s\n", CCERT_PKEY_FILE);
                }
            }
            printf("name = %s, header = %s, len = %d\n", name, header, len);
            free(name);
            free(header);
            free(data);
        } while(1);
    } while(0);

    if (fp_in)
        fclose(fp_in);

    if (fp_key)
        fclose(fp_key);

    if (fp_cert)
        fclose(fp_cert);
    return 0;
}

int main()
{
    spilt_spem(PEM_FILE);
    return 0;
}
