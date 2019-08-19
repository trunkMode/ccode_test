#include <unistd.h>

void strncpy_test()
{
	char a[10];
	char *p = "world hello";

	strncpy(a, "hello", sizeof(a));
	strncpy(a + strlen(a), p, sizeof(a) - strlen(a) - 1);

	printf("a = %s\n", a);	
}
int main()
{
#if 0
	strncpy_test();
#endif
#if 1
	if (strcmp(NULL, "xx", strlen("xx")) == 0) {
		printf("xxxx\n");
	} else {
		printf("yyyy\n");
	}
#endif
}


