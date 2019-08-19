#include <unistd.h>


int main()
{
	if (memcmp("test", NULL, 0)) {
		printf("not equal \n");
	} else {
		printf("equal ...\n");
	}
}
