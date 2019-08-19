#include <unistd.h>

int main()
{
	char buf[32] = {'a', 0};
	char *p = NULL;
	snprintf(buf, sizeof(buf), "%s", p);
	printf("buf = %s\n", buf);
	printf("hello world\n");

	return 0;
}
