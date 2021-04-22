#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdio.h>

void sysinfo_test()
{
    struct sysinfo sys_info;
#if 0
           struct sysinfo {
               long uptime;             /* Seconds since boot */
               unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
               unsigned long totalram;  /* Total usable main memory size */
               unsigned long freeram;   /* Available memory size */
               unsigned long sharedram; /* Amount of shared memory */
               unsigned long bufferram; /* Memory used by buffers */
               unsigned long totalswap; /* Total swap space size */
               unsigned long freeswap;  /* swap space still available */
               unsigned short procs;    /* Number of current processes */
               char _f[22];             /* Pads structure to 64 bytes */
           };
#endif
    if (!sysinfo(&sys_info)) {
        printf("uptime = %ld\n", sys_info.uptime);
        printf("totalram = %d\n", sys_info.totalram);
        printf("freeram = %d\n", sys_info.freeram);
        printf("bufferram = %d\n", sys_info.bufferram);
        printf("totalswap = %d\n", sys_info.totalswap);
        printf("freeswap = %d\n", sys_info.freeswap);
        printf("procs = %d\n", sys_info.procs);
    }
}

int main()
{
	sysinfo_test();
	return 0;
}
