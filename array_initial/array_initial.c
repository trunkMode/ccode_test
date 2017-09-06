#include <unistd.h>

void main(void)
{
	char *p[10] ={
		[0] = "hello world\n",
		[1 ... 8] = NULL,
        [9] = "test\n",
		
	};
	char str[2][3] ={0};
	printf("%s %d: p[0] = %s\n", __func__, __LINE__, p[0]);
	
	printf("sizeof(str) = %d\n", sizeof(str));
	printf("sizeof(str[0]) = %d\n", sizeof(str[0]));
}
