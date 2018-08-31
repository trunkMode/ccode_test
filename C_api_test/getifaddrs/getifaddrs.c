#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#if 0
       #include <sys/types.h>
       #include <ifaddrs.h>

       int getifaddrs(struct ifaddrs **ifap);

              void freeifaddrs(struct ifaddrs *ifa);


       #include <arpa/inet.h>

	             const char *inet_ntop(int af, const void *src,
				            char *dst, socklen_t size);

#endif

int main()
{
    int af = AF_INET;
    char addr_str[32];
	struct ifaddrs *ifap = NULL, *tmp_ifap;

	if (getifaddrs(&ifap) < 0) {
		perror("getifaddrs failed:");
		return -1;
	}

	tmp_ifap = ifap;
	while (tmp_ifap) {
		printf("ifa_name: %s\n", tmp_ifap->ifa_name);
		printf("ifa_flags: %x\n", tmp_ifap->ifa_flags);
	
	    if (inet_ntop(tmp_ifap->ifa_addr->sa_family, tmp_ifap->ifa_addr, addr_str, sizeof(addr_str))) {
	        printf("addr: %s\n", addr_str);
	    }
	    
	    if (tmp_ifap->ifa_netmask && inet_ntop(tmp_ifap->ifa_netmask->sa_family, tmp_ifap->ifa_netmask, addr_str, sizeof(addr_str))) {
	        printf("ifa_netmask: %s\n", addr_str);
	    }
        printf("\n\n");
        tmp_ifap = tmp_ifap->ifa_next;
	}
}



