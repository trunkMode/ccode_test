#include <unistd.h>


int main()
{
	char str1[5][32];
	char *str = "test hello world";

	memset(&str1[0][0], 0x00, sizeof(char)*5*32);
	int ret = sscanf(str, "%s %s %s %s", &str1[0][0], &str1[1][0], &str1[2][0], &str1[3][0]);
	
	int i;

	for (i = 0; i < 5; i++) {
		printf("str = %s\n", &str1[i][0]);
	}

	return 0;
}
