#include <mcheck.h>
#include <unistd.h>

int two_dimension_array_test()
{
	int i,j;
	char b[3][10];

	memset(b, 0x8, 3*10);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 10; j++) {
			printf("b[%d][%d] = %d\n", i, j, b[i][j]);
		}
	}
}

int crash_test()
{
	char *p = NULL;
	p = malloc(100);
	*p= 0x88;
	return 0;
}

int main()
{
 	char a[6] = {0x00};
	char *p = &a[0];
	char *q = &p;

	mtrace();
	
	printf("&a = %p, &a[0] = %p, &&a[0] = %p\n", &a, &a[0], &a);
	printf("p = %p, q = %p\n");

	two_dimension_array_test();
	
	crash_test();

	printf("alan update\n");
	printf("alan add 20171013\n");

	printf("hgr update 20171013 1:00\n");
	printf("alan add 20171016_1:31\n");
	printf("alan add \n");

}
