#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#if 0
void develop(EVP_PKEY* priKey,BYTE** out,unsigned int &cOut, BYTE* data,unsigned int cData){  
    OpenSSL_add_all_algorithms();  
    bool hasErr = true;  
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(priKey,NULL);  
    if(!ctx)  
        goto err;  
    if (EVP_PKEY_decrypt_init(ctx) <= 0)  
        goto err;  
    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)//与加密同样的PKCS1补齐   
        goto err;  
    if (EVP_PKEY_decrypt(ctx, NULL, &cOut, data, cData) <= 0)//解密结果大小   
        goto err;  
    *out =(BYTE*) OPENSSL_malloc(cOut);//分配内存   
    if (!out)  
        goto err;  
    if (EVP_PKEY_decrypt(ctx, *out, &cOut, data, cData) <= 0)//得到解密结果   
        goto err;  
    hasErr = false;  
err://输出一些错误信息,偶尔错误信息得不到,只得到一些文件名和所在行,就跑到源代码去找...麻烦死了,如果有好的方案求回复~   
    EVP_PKEY_CTX_free(ctx);  
    if(!hasErr)  
        return;  
    cout << "err in develop" << endl;  

    const      char*file,*data1,*efunc,*elib,*ereason,*p;  

    int                         line,flags;  
    unsigned long  errn;  


    errn=ERR_peek_error_line_data(&file,&line,&data1,&flags);  

    printf("ERR_peek_error_line_data err : %ld,file :%s,line :%d,data :%s\n",errn,file,line,data1);  


}
#endif

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

RSA *read_RSA_key_from_file(const char *key_file, int is_public)
{
    RSA *rsa_pkey = NULL;
    FILE *fp = NULL;

    do {
        f (!(fp = fopen(key_file, "r"))) {
            printf("can't fopen private key file \n");
            return NULL;
        }
        PEM_read_RSAPrivateKey(fp, &rsa_pkey, NULL, NULL);

        printf("RSA_size(rsa) = %d\n", RSA_size(rsa_pkey));
    } while (0);

    fclose(fp);

    return rsa_pkey;
}


int main()
{
    int i;
    uint8_t data[1024*5] = {0}, out[1024*5] = {0};

    RSA *rsa_pkey = read_RSA_key_from_file("test.key", 0);
    int ret = read_file("data.bin", data, sizeof(data));

    printf("ret = %d\n", ret);
    ret = RSA_private_decrypt(256, data, out, rsa_pkey, RSA_NO_PADDING/*RSA_PKCS1_PADDING*/);
    printf("ret = %d\n", ret);

    char err_buf[1024];
    unsigned long err = ERR_get_error();
    ERR_error_string(err, err_buf);
    printf("error = %s\n",err_buf);
//  int RSA_private_decrypt(int flen, const unsigned char *from,
//                                            unsigned char *to, RSA *rsa, int padding);

    for (i = 0; i < 256; i++) {
        printf("%02x ", (uint8_t)out[i]);
        if (i % 15 == 0)
            printf("\n");
    }
    return 0;
}
