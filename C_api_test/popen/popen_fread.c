#include <unistd.h>
#include <stdio.h>

int main()
{
	char buf[1024*1024*5];
//	FILE *fp = popen("curl -k -X GET -s http://software.sonicwall.com/applications/sonicpoint/sp_sm_8.2.1.0_1.bin.sig1", "r");
	FILE *fp = popen("cat" , "r");

	if (!fp)
		return;
	int n = fread(buf, 1, 1024*1024*5, fp);
	printf("n = %d (buflen is %d\n", n, 1024*1024*5);	
}
