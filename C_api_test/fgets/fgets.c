#include <unistd.h>
#include <stdio.h>

int main()
{
	FILE *fp = fopen("test.txt", "r");
	char buf[64], str[32];

	if (!fp)
		return -1;

	if (fgets(buf, sizeof(buf), fp)) {
        sscanf(buf, "ip=%s\n", str); 
        printf("str = %s_\n",str);
		printf("buf = %s,strlen(buf) is %d\n", buf, strlen(buf));
	}
	return 0;
}	
