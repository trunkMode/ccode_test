#include <unistd.h>

union {
	unsigned char a;
	unsigned char a1:1;
	unsigned char a2:1;
	unsigned char a3:1;
	unsigned char a4:1;
	unsigned char a5:1;
	unsigned char a6:1;
	unsigned char a7:1;
	unsigned char a8:1;
} test;


int main()
{
	test.a = 0x0c;
	printf("a = 0x%x\n", test.a);
	printf("a1 = 0x%x\n", test.a1);
	printf("a2 = 0x%x\n", test.a2);
	printf("a3 = 0x%x\n", test.a3);
	printf("a8 = 0x%x\n", test.a8);
	return 0;
}
