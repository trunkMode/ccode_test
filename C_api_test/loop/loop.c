#include <unistd.h>
#include "test.h"
int main()
{
	int i,j = 10;
	for (i = 0; i < 10; i++) {
		j = 2;
		do {
			printf(" i = %d, yyyyy\n", i);
			if (i < 5)
				continue;
			printf("xxxxxxxxx\n");
			j--;

		} while (j > 0);
	}
}

