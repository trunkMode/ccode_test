#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int ath10k_parse_param_list(const char *param_list, const char *key, char *value_buf, int buf_len)
{
    int ret = -1, k_len = 0; 
    char *dst;
    const char param_sep = ';', kv_sep = '='; 
    const char *sep, *sep_kv, *list = param_list;

    if (!key || !param_list || !value_buf || !buf_len)
        return ret; 

    while ((sep = strchr(list, param_sep))) {
        printf("sep = %c\n", *sep);
        list = sep + 1;
        printf("list = %s\n", list);
        sep_kv = strchr(sep + 1, kv_sep);
        if (!sep_kv)
            break;

        k_len = (sep_kv - (sep + 1)); 
        if (strlen(key) == k_len &&
            !memcmp(sep + 1, key, k_len)) {
            sep_kv++;

            k_len = 0; 
            dst = value_buf;
            while (*sep_kv != '\0' && *sep_kv != param_sep) {
                k_len++;
                if (k_len >= (buf_len - 1)) {
                    k_len = buf_len  - 1; 
                    break;
                }    
                *dst++ = *sep_kv++;
            }    
            value_buf[k_len] = '\0';
            printf("xxx = %s\n", value_buf);
        }
    }
    return ret;
}
int main()
{
    char value_buf[128];
//    const char *param_list = "US;test=1;product=570w;hello=test";
    const char *param_list = ";;;;;;test=1;product=570w;hello=test";
    ath10k_parse_param_list(param_list, "product", value_buf, sizeof(value_buf));
//    ath10k_parse_param_list(const char *param_list, const char *key, char *value_buf, int buf_len)
    printf("value_buf = %s\n", value_buf);
}
