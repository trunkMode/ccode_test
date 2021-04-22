#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t wlan0Mac[6] = {0xc0, 0x00, 0xff,0x00, 0x00, 0x00};
uint8_t wlan1Mac[6] = {0xc0, 0x00, 0xff,0x00, 0x10, 0x00};

#define ENET_ADDR_SIZE 8
#define SP_INVALID_MAC 0
#define WLAN_MAX_VAP   8
#define MACPARAMS(a) (a)[0],(a)[1],(a)[2],(a)[3],(a)[4],(a)[5],(a)[6]


uint64_t
sonicPointGetHexMac(uint8_t *mac)
{
	uint64_t hexMac = 0;
	char macStr[16];
	
	snprintf(macStr, ENET_ADDR_SIZE * 2 + 1, 
		"%02x%02x%02x%02x%02x%02x", MACPARAMS(mac));
	hexMac = strtoull(macStr, (char **)NULL, 16);
	return hexMac;
}

int is_own_vap_mac(uint8_t *mac)
{
    uint64_t hexmac = sonicPointGetHexMac(mac);
    uint64_t hexmac0 = sonicPointGetHexMac(wlan0Mac);
    uint64_t hexmac1 = sonicPointGetHexMac(wlan1Mac);

    if ((hexmac >= hexmac0 && hexmac <= hexmac0 + WLAN_MAX_VAP - 1) ||
        (hexmac >= hexmac1 && hexmac <= hexmac1 + WLAN_MAX_VAP - 1)) {
        return 1;
    }
    return 0;
}

int main()
{
	uint8_t mac[] = { 0xc0, 0x00, 0xff, 0x00, 0x00, 0x08};

	printf("%d\n", is_own_vap_mac(mac));
}
