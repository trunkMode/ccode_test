#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#define CMD_MAX_LEN     2048
int runCmd(const char * format, ...)
{
    va_list arg;
    int ret = -1;
    char *cmd = NULL;
    cmd = malloc(CMD_MAX_LEN);
    if(cmd)
    {
        memset(cmd, 0, CMD_MAX_LEN);
        va_start(arg, format);
        vsprintf(cmd, format, arg);
        va_end(arg);

        ret = system(cmd);
#if 0 /* enable it when debugging cmd */
        printf("\n[%s] (%d)\n", cmd, ret);
#endif
        free(cmd);
    }
    return ret;
}

static int sp_sync_process()
{
#define SEM_SYNC                    "/SEM_SYNC"
#define SEM_MAX_TIMEOUT             20
    const char *proc_name[] = {
        "spMonDaemon",
        /*new process continue*/
        NULL,
    };

    int i;
    int sync_proc_cnt = sizeof(proc_name)/sizeof(char*);
    sem_t *sync_sem;
    struct timespec tv;

    if ((sync_sem = sem_open(SEM_SYNC, O_CREAT, 0600, 0)) == SEM_FAILED) {
        sem_unlink(SEM_SYNC);
        return -1;
    }
    sleep(5);
    tv.tv_sec = time(NULL) + SEM_MAX_TIMEOUT;
    tv.tv_nsec = 0;

    for (i = 0; i < sync_proc_cnt && proc_name[i]; i++) {
        runCmd("killall -SIGUSR1 %s", proc_name[i]);
    }

    printf("sync_proc_cnt = %d\n", sync_proc_cnt);
    while (sync_proc_cnt--) {
        if (sem_timedwait(sync_sem, &tv) < 0) {
            perror("sync_sem:");
            break;
        }
    }
    sem_close(sync_sem);
    sem_unlink(SEM_SYNC);
    return 0;
}

int main()
{
    sleep(5);
    sp_sync_process();
    return 0;
}
