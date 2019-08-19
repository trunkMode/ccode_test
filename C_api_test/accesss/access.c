#include <unistd.h>

int main()
{
	if (access("*.c", F_OK)== 0)
		printf("*.c exist %s\n", NULL);
	else
		printf("*.c don't exist %s\n", NULL);
}
