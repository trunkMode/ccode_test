#include <unistd.h>

int main()
{
	
	int unit, bss;
	int ret = sscanf("br12", "br%1d%1d", &unit, &bss);
	
	printf("ret = %d,unit = %d, bss = %d\n", ret, unit, bss);
}
