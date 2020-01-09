#include <unistd.h>

int test(char *p) {
    int i;
    for (i = 0; i < 20000; i++)
        printf("p = 0x%x\n", p[i]);
    return 0;
}
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
