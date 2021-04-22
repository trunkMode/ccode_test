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
	struct sockaddr_in *sk_addr;

	if (getifaddrs(&ifap) < 0) {
		perror("getifaddrs failed:");
		return -1;
	}

	tmp_ifap = ifap;
	while (tmp_ifap) {
		printf("ifa_name: %s\n", tmp_ifap->ifa_name);
		printf("ifa_flags: %x\n", tmp_ifap->ifa_flags);
		
		sk_addr = (struct sockaddr_in *) tmp_ifap->ifa_addr;
	    printf("ifa_addr: %0x\n", ntohl(sk_addr->sin_addr.s_addr));
	    
	    if (tmp_ifap->ifa_addr) {
	        if (tmp_ifap->ifa_addr->sa_family == AF_INET && 
	           inet_ntop(AF_INET, &(((struct sockaddr_in *)tmp_ifap->ifa_addr)->sin_addr), addr_str, sizeof(addr_str))) {
	            printf("addr: %s\n", addr_str);
	        } else if (tmp_ifap->ifa_addr->sa_family == AF_INET6 &&
	            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)tmp_ifap->ifa_addr)->sin6_addr), addr_str, sizeof(addr_str))) {
	            printf("addr: %s\n", addr_str);
	        }
	    }
	    
	    if (tmp_ifap->ifa_netmask) {
	        if (tmp_ifap->ifa_netmask->sa_family == AF_INET &&
	            inet_ntop(AF_INET, &(((struct sockaddr_in *)tmp_ifap->ifa_netmask)->sin_addr), addr_str, sizeof(addr_str))) {
	            printf("netmask: %s\n", addr_str);
	        } else if (tmp_ifap->ifa_addr->sa_family == AF_INET6 &&
	            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)tmp_ifap->ifa_netmask)->sin6_addr), addr_str, sizeof(addr_str))) {
	            printf("netmask: %s\n", addr_str);
	        }
	        
	    }

        printf("\n\n");
        tmp_ifap = tmp_ifap->ifa_next;
	}
	freeifaddrs(tmp_ifap);

}



