#include <unistd.h>  
#include <openssl/sha.h>
#include <stdint.h>
#include <sys/time.h>

int get_random()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((unsigned int) tv.tv_usec);
    return random();
}

int gen_sha256_nonce(const unsigned char *mac, size_t mac_len, int boardId, unsigned char *nonce, int *nonce_len)
{
    unsigned char *d = NULL;
    int value, len;

    if (!(d = malloc(mac_len + sizeof(int) * 2))) {
        return -1;
    }
    len = 0;
    memcpy(d, mac, mac_len);
    len += mac_len;

    value = htonl(boardId);
    memcpy(d + len, &value, sizeof(int));
    len += sizeof(int);

    value = htonl(get_random());
    memcpy(d + len, &value, sizeof(int));
    len += sizeof(int);

    SHA256(d, len, nonce);
    *nonce_len = SHA256_DIGEST_LENGTH;
    
    return 0;
}
//const unsigned char *mac, size_t mac_len, int boardId, unsigned char *nonce, int *nonce_len)
void get_nonce_test()
{
    int len;
    unsigned char nonce[32*2+1];
    char nonce_str[128] = {0};
    int i, nonce_len = sizeof(nonce);
    uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    
    gen_sha256_nonce(mac, sizeof(mac), i, nonce, &nonce_len);
        
    len = 0;
    for (i = 0; i < nonce_len; i++) {
        snprintf(nonce_str + len, sizeof(nonce) - len, "%02x ", nonce[i]);   
        len += 2;
    }
    
    printf("nonce_str = %s\n", nonce_str);
        printf("\n");
    /* restore nonce from string */
    memset(nonce, 0x00, sizeof(nonce));
    len = 0;
    for (i = 0; i < 32; i++) {
        sscanf(nonce_str + len, "%02x", &nonce[i]);
        len += 2;
    }
    
    printf("\n");
    for (i = 0; i < 32; i++) {
        printf(" %02x", nonce[i]);
    }
        printf("\n");
}
int main()  
{  
    unsigned char md[128] = {0};  
    SHA256((const unsigned char *)"hello1", strlen("hello1"), md);  
      
    int i = 0;  
    char buf[65] = {0};  
    char tmp[8] = {0};  
    for(i = 0; i < 32; i++ )  
    {  
        sprintf(tmp,"%02X", md[i]);  
	strcat(buf, tmp);  
        
	md[i] ^= 'x';
        sprintf(tmp," %02X ", md[i]);  
	strcat(buf, tmp);  
    }  
  
    printf("buf = %s\n", buf);
  
    for (i = 0; i< 5; i++)
        get_nonce_test();
    return 0;  
} 
