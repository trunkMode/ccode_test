#include <unistd.h>
#include <sys/time.h>
#include <time.h>


int main()
{
    struct tm *tm;
    time_t ts = time(NULL);
    printf("time = %d\n", ts);
    printf("ctime = %s\n", ctime(&ts));

    tm = localtime(&ts);
    printf("%d/%d/%d %d:%d:%d\n", tm->tm_year + 1900,
           tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    /* gettimeofday */
//int gettimeofday(struct timeval *tv, struct timezone *tz);
//int settimeofday(const struct timeval *tv, const struct timezone *tz);

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	printf("%d/%d/%d %d:%d:%d\n", tm->tm_year + 1900, 
		   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	printf("tz_minuteswest = %d, tz_dsttime = %d\n", tz.tz_minuteswest, tz.tz_dsttime);


	tz.tz_minuteswest = 120;
	settimeofday(NULL, &tz);


	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	printf("%d/%d/%d %d:%d:%d\n", tm->tm_year + 1900, 
		   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	printf("tz_minuteswest = %d, tz_dsttime = %d\n", tz.tz_minuteswest, tz.tz_dsttime);
#if 0
    int i;

    for (i = 0; i < 0; i++)
        printf("for############\n");
    const char *p = "12.53%, 11.22%";
    float cpu = 0;
    sscanf(p, "%f%%", &cpu);
    printf("cpu = %1.2f%%\n", cpu);
#endif
	sleep(1000);
}
