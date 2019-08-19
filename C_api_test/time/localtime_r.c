#include <time.h>

int main()
{
	time_t ts = time(NULL);
	struct tm tm;
	char buf[512];

	memset(&tm, 0x00, sizeof(tm));

    if (ctime_r(&ts, buf)) {
        printf("len = %d, strlen = %d\n", sprintf(buf, "%s", buf), strlen(buf)); 
        printf("buf = %s\n", buf);
    }
	//asctime_r(NULL, buf);
	if (localtime_r(&ts, &tm) && asctime_r(&tm, buf)) {
		printf("buf = %s\n", buf);
	}

	if (gmtime_r(&ts, &tm) && asctime_r(&tm, buf)) {
		printf("buf_gm = %s\n", buf);
	}


}
