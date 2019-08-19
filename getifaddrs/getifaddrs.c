#include <sys/types.h>
#include <ifaddrs.h>

int main()
{
	struct ifaddrs *ifap = 0;

	if (getifaddrs(&ifap) < 0){
		perror("xxx:");
		return -1;
	}
	freeifaddrs(ifap);
	return 0;
}
