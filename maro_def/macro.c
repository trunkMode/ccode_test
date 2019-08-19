#include <stdio.h>

#define STR(x)  #x
#define conn(y) x##y 

int main()
{
	char *str1 = STR(hello);
	char *str = conn("hello");
	printf("%s \n", str);
}
