#include <unistd.h>
#include <stdio.h>

int stack_overflow_test()
{
	int i;
	char array[1],array1[1];
	
	gets(array);

	
	for (i = 0; i < sizeof(array1); i++) {
		printf("array1[%d] = %c\n", i, array1[i]);
	}
	sleep(1);
	return 0;
}

int main()
{
	stack_overflow_test();
	sleep(1);
	return 0;
}
