#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define INCLUDE_SONICPOINT  1
#define SP_VPN_CONFIG   "vpn_cfg.txt"
typedef struct {
    char user[128];
    char passwd[128];
    unsigned char *enc_passwd;
    size_t enc_passwd_len;
    char domain[128];
} sp_vpn_cfg_t;

#define STRIP_END_CHAR(str,c) {         \
        int __len = strlen((str));          \
        if ((str)[__len - 1] == (c)) {      \
                    (str)[__len - 1] = '\0';        \
                }                                   \
}

#if 0
static sp_vpn_cfg_t *load_config_from_file(sp_vpn_cfg_t *cfg)
{
    int cnt = 0; 
    FILE *fp = NULL;
    char line[512];

    do { 
        if (!(fp = fopen(SP_VPN_CONFIG, "r"))) {
            printf("Failed to open file\n");
            break;
        }

        while (fgets(line, sizeof(line), fp)) {
            printf("line = %s\n", line);
            STRIP_END_CHAR(line, '\n');
            printf("line = %s\n", line);
            if (sscanf(line, "user=%s", cfg->user) == 1) { 
                cnt++;
            } else if (sscanf(line, "passwd=%s", cfg->passwd) == 1) { 
                cnt++;
            } else if (sscanf(line, "domain=%s", cfg->domain) == 1) { 
                cnt++;
            }
            if (cnt >= 3)
                break;
            //*encryptedPwd = aesEncrypt((unsigned char *) optarg, strlen(optarg),
            //getKeyData(), KEY_DATA_SIZE, NULL, encryptedPwdLen);
            ////        nxlogTrace(NXLOG_AUTH, "*encryptedPwdLen = %zd", *encryptedPwdLen);
        }
    } while (0); 
    if (fp) 
        fclose(fp);
    return (cnt >= 3 ? cfg : NULL);
}
#else
static sp_vpn_cfg_t *get_config_from_file()
{
    int cnt = 0;
    FILE *fp = NULL;
    char line[512];
    sp_vpn_cfg_t *cfg = NULL;

    do {
        if (!(fp = fopen(SP_VPN_CONFIG, "r"))) {
            //nxlogError(NXLOG_CONNECT, "can't open cfg file\n");
            break;
        }

        if (!(cfg = malloc(sizeof(*cfg)))) {
            //nxlogError(NXLOG_CONNECT, "can't allocate mem for reading cfg\n");
            break;
        }
        memset(cfg, 0x00, sizeof(*cfg));

        while (fgets(line, sizeof(line), fp)) {
            STRIP_END_CHAR(line, '\n');
            if (sscanf(line, "user=%s", cfg->user) == 1) {
                cnt++;
            } else if (sscanf(line, "passwd=%s", cfg->passwd) == 1) {
                cfg->enc_passwd = NULL;
              // cfg->enc_passwd = aesEncrypt((unsigned char *)cfg->passwd, strlen(cfg->passwd),
              //          getKeyData(), KEY_DATA_SIZE, NULL, &cfg->enc_passwd_len);
              //  if (cfg->enc_passwd)
                    cnt++;
            } else if (sscanf(line, "domain=%s", cfg->domain) == 1) {
                cnt++;
            }
            if (cnt >= 3) {
                //nxlogInfo(NXLOG_CONFIG, "%s", "reading cfg successfully");
                break;
            }
        }
    } while (0);

    if (fp)
        fclose(fp);

    if (cfg < 3) {
        printf("Failed to get cfg \n");
        /* error */
        free(cfg);
        cfg = NULL;
    }

    return cfg;
}

static void free_config_from_file(sp_vpn_cfg_t *cfg)
{
    if (!cfg)
        return;
    if (cfg->enc_passwd) {
        free(cfg->enc_passwd);
    }    
    free(cfg);
}

#endif

int main()
{
    sp_vpn_cfg_t *cfg;
    cfg = get_config_from_file();

    printf("user = %s_\n", cfg->user);
    printf("passwd = %s_\n", cfg->passwd);
    printf("domain = %s_\n", cfg->domain);
    
    free_config_from_file(cfg);
    return 0;
}
