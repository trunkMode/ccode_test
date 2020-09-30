#include <unistd.h>

int main()
{
	int unit, bss = 0, vid = 0;
	int ret = sscanf("br12", "br%1d%1d.%1d", &unit, &bss, &vid);
//	int ret = sscanf("brvlan.12", "%d", &bss);
    printf("isdigit('0') = %d\n", isdigit('a'));
	printf("ret = %d,unit = %d, bss = %d, vid = %d\n", ret, unit, bss, vid);
}
