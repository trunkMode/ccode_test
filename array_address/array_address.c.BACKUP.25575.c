#include <unistd.h>

int main()
{
 	char a[6] = {0x00};
	char *p = &a[0];
	char *q = &p;
	
	printf("&a = %p, &a[0] = %p, &&a[0] = %p\n", &a, &a[0], &&a[0]]);
	printf("p = %p, q = %p");
	
	printf("alan update\n");
	printf("alan add 20171013\n");
<<<<<<< HEAD
	printf("alan add 20171016_1:31\n");
	printf("alan add \n");
=======
	printf("hgr update 20171013 1:00\n");
>>>>>>> e1940d274f3c2f1989227bf80a20ee7a541c02cc
}
