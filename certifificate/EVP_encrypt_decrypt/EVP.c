/*
 * @func decryt data which encrypted by public key from ct server
 * @cmd: openssl rsautl -raw -decrypt -in data.bin -inkey client.key -out data.bin.dec
 * 
 */
#include <openssl/evp.h>
#include <openssl/rsa.h>

#define CT_ERROR 0
#define CT_DEBUG_LOG(level,format,arg...)      printf(format"\n", ##arg)

/* EVP_PKEY_free */
EVP_PKEY *get_evp_pkey(const char *pkey_file)
{
    FILE *fp = NULL;
    EVP_PKEY *pkey = NULL;

    do {
        if (!(fp = fopen(pkey_file, "r"))) {
            printf("Failed to open file %s\n", pkey_file);
            break;
        }
        OpenSSL_add_all_algorithms();
        pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
        if (!pkey)
            printf("failed to read private key\n");

        printf("type = %d\n",  EVP_PKEY_base_id(pkey));
    } while(0);

    if (fp)
        fclose(fp);

    return pkey;
}

size_t evp_decrypt(unsigned char *in, size_t inlen, unsigned char **out)
{
    size_t outlen = 0;
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *pkey = NULL;
    /* NB: assumes key in, inlen are already set up
     * and that key is an RSA private key
     */

    do {
        if (!out)
            break;
        if (!(pkey = get_evp_pkey("client.key"))) {
            printf("Failed to load private key.\n");
            break;
        }

        ctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ctx) {
            printf("fail to create pkey ctx \n");
            break;
        }

        if (EVP_PKEY_decrypt_init(ctx) <= 0) {
            printf("decrypt init fail.\n");
            break;
        }

        if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING) <= 0) {
            printf("Failed to set rsa padding \n");
            break;
        }

        /* Determine buffer length */
        if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
            printf("Failed to determine buffer length \n");
            break;
        }

        /* prepare memory */
        *out = malloc(outlen);
        if (!*out) {
            printf("fail to allocate mem.\n");
            break;
        }

        if (EVP_PKEY_decrypt(ctx, *out, &outlen, in, inlen) <= 0) {
            printf("Fail to decrypt \n");
            break;
        }

        /* Decrypted data is outlen bytes written to buffer out */
        for (int i = 0; i < outlen; i++) {
            printf("%02x ", (*out)[i]);
        }
    } while (0);

    EVP_PKEY_free(pkey);
    return outlen;
}

int read_file(const char *filename, char *buf, unsigned int len)
{
    FILE *fp = NULL;
    size_t ret = 0;

    if (!buf || len == 0) {
        return -2; 
    }   
    if ((fp = fopen(filename, "rb")) == NULL) {
        return -1; 
    }   

    memset(buf, 0x00, len);
    while ((ret += fread(buf + ret, 1, len, fp)) < len) {
        if (ferror(fp) || feof(fp))
            break;
    }   
    fclose(fp);
    return ret;
}

int sign_by_pkey(const char *pkey_file, uint8_t *sign_data, size_t sdata_len, uint8_t **out)
{
    size_t sign_len = 0;
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *pkey = NULL;

    do {
        if (!out)
            break;

        if (!(pkey = get_evp_pkey(pkey_file))) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to load private key.");
            break;
        }

        ctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ctx) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to create pkey ctx");
            break;
        }

        if (EVP_PKEY_sign_init(ctx) <= 0) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to do sign init");
            break;
        }
//        int EVP_PKEY_CTX_get_signature_md(EVP_PKEY_CTX *ctx, const EVP_MD **pmd);
        EVP_MD *md_ptr = NULL; 
        int md_type = EVP_PKEY_CTX_get_signature_md(ctx, &md_ptr);
        
        printf("md_type = %d%s\n", md_type, md_ptr ? "not null" :"null");//EVP_MD_type(md_ptr));
#if 0
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to set rsa padding.");
            break;
        }
#endif
        if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0) {
            CT_DEBUG_LOG(CT_ERROR, "Fail to set signature md");
            break;
        }

        if (EVP_PKEY_sign(ctx, NULL, &sign_len, sign_data, sdata_len) <= 0) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to get sign len");
            break;
        }

        *out = malloc(sign_len);
        if (!*out) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to allocate mem for sign");
            break;
        }

        if (EVP_PKEY_sign(ctx, *out, &sign_len, sign_data, sdata_len) <= 0) {
            CT_DEBUG_LOG(CT_ERROR, "Failed to sign with pkey");
            break;
        }


    } while (0);

    EVP_PKEY_free(pkey);
    printf("sign_len = %d\n", sign_len);

    for (int i = 0; i < sign_len; i++) {
        printf("%02x ", (*out)[i]);
    }
    return 0;
}

int main()
{
    uint8_t *out = NULL;
    uint8_t data[1024*5] = {0};
    int ret = read_file("data.bin", data, sizeof(data));
printf("ret = %d\n", ret);
#if 0
//    evp_decrypt(unsigned char *in, size_t inlen, unsigned char **out)
//    evp_decrypt(data, ret, &out);
#else
// int sign_by_pkey(const char *pkey_file, uint8_t *sign_data, size_t sdata_len, uint8_t **out) 
    sign_by_pkey("client_secp384r1.key", data, ret, &out);

#endif
    return 0;
}
