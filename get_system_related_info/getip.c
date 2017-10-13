#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "getip.h"
void get_arp_ip()
{
    char *str = "10.11.12.1";
    char *p = str;
    char ips[16][16] = {0};

        sscanf(str,"%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];", ips[0],ips[1],ips[2],ips[3],ips[4],ips[5],ips[6],ips[7],ips[8],ips[9],ips[10],ips[11],ips[12],ips[13],ips[14],ips[15]);
    int i,j;

    for (i = 0; i < 16; i++)
        printf("ips[%d] = %s, strlen(ips[%d])= %d, 0x%x\n", i, &ips[i][0], i,strlen(ips[i]), inet_addr(ips[i]));
}

void main()
{
	int txpower = 3;
	get_arp_ip();
	printf("%d\n", txpower > 0 ?:88);
	printf("ABC = %d\n", ABC);
}
