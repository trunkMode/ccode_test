#include <unistd.h>


int main()
{
	int i;
	sleep(10);
	srandom(0);

	for (i = 0; i < 10; i++) {
		printf("%ld\n", random());
		printf("%ld\n", rand());
		printf("%ld\n", random());
		printf("%ld\n", rand());
	}
}
