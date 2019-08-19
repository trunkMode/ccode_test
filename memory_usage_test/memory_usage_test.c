#include <unistd.h>


int main()
{
	printf("malloc 20M \n");
	char *p = malloc(20*1024*1024);
	sleep(20);

	printf("malloc 20M again\n");
	char *q = malloc(20*1024*1024);
	sleep(20);

	printf("free 20M \n");
	free(p);
	sleep(20);

	printf("free 20M again\n");
	free(q);

	printf("no mem used\n");
	sleep(20);

}
