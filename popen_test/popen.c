#include <unistd.h>
#include <stdio.h>

int main()
{
	char buf[12800];
	FILE *fp = popen("traceroute www.baidu.com", "r");
#if 0
	while (1) {
		printf("xxxxxxxx\n");
		int ret = fread(buf, 1, sizeof(buf), fp);
		printf("ret = %d\n",ret);
	}
#endif
	if (!fp) {
		printf("popen failed\n");
	} else {
		printf("open success\n");
	}

	while (fgets(buf, sizeof(buf), fp)) {
		printf("buf = %s\n", buf);
		sleep(1);
	}
}
