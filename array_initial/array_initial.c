#include <unistd.h>

typedef enum {
	TYPE_A,
	TYPE_B,
	TYPE_C,
	TYPE_MAX,
};
void main(void)
{
	int i;
	int test[TYPE_MAX] = {11,22,33};
	char *p[10] ={
		[0] = "hello world\n",
		[1 ... 8] = NULL,
        [9] = "test\n",
		
	};
	char str[2][3] ={0};
	printf("%s %d: p[0] = %s\n", __func__, __LINE__, p[0]);
	
	printf("sizeof(str) = %d\n", sizeof(str));
	printf("sizeof(str[0]) = %d\n", sizeof(str[0]));

	printf("TYPE_MAX = %d\n", TYPE_MAX);

	for (i = 0; i < sizeof(test)/sizeof(int); i++) {
		printf("test[%d] = %d\n", i, test[i]);
	}	
}
