#include <unistd.h>

int main()
{
	char ssid[10][32];
	int unit, bss;

	unit = bss = 8;
	int ret = sscanf("tf.1", "%*[^0-9]%d.%d", &unit, &bss);
	printf("ret = %d, unit = %d, bss = %d \n",ret, unit, bss);

	unit = bss = 8;
	ret = sscanf("tf2.3", "%*[^0-9]%d.%d", &unit, &bss);
	printf("ret = %d, unit = %d, bss = %d\n",ret, unit, bss);

	printf("sizeof(ssid) = %d\n", sizeof(ssid));
	return 0;
}
