#include <stdlib.h>

int main()
{
	char *endp = NULL;
	char *ptr = "7";
	int ret = strtol(ptr, &endp, 10);
	printf("ret = %d, endp = %p, ptr = %p\n", ret, endp, ptr);
printf("xxxx%s\n", TEST);
	return 0;
}
