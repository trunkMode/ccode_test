#include <unistd.h>
#include <stdio.h>

#define CMD_MAX_ARG_NUM 10
#define CMD_MAX_ARG_LEN  64
int parseCmd(char cmd[], char argv[][CMD_MAX_ARG_LEN]);

int main()
{
    char cmd[] = "/opt/sonicwall/sonicwave/bin/enableRadioCfg";
    char argv[CMD_MAX_ARG_NUM][CMD_MAX_ARG_LEN]; 

    parseCmd(cmd, argv);
}

int parseCmd(char cmd[], char argv[][CMD_MAX_ARG_LEN])
{
    char *arg, *token = NULL, *save_ptr= NULL;
    int argc = 0;
           
    if (strlen(cmd) <= 0) {
        return -1;
    }      
           
    arg = strtok_r(cmd, "/", &save_ptr);
    if (arg) {
        token = arg;
        while (token) {
            arg = token;
            token = strtok_r(NULL, "/", &save_ptr);
        }  
    }      
           
    save_ptr = NULL;
    arg = strtok_r(arg, " ", &save_ptr);                                                                                                                                            
    while (arg && argc < CMD_MAX_ARG_NUM) {
        memcpy(argv[argc++], arg, strlen(arg) + 1);
        arg = strtok_r(NULL, " ", &save_ptr);
    }      
           
    return argc;
} 
