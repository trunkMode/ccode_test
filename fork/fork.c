#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

void sig_handler(int signum)
{
    printf("rcv sig:%d\n", signum);
}

int main()
{
    pid_t pid, pid_wait;
    int wstatus = 0;

    /* signal */
    sigset_t nmask;
    sigset_t omask;

    signal(SIGCHLD, sig_handler);
#if 0
    sigemptyset(&nmask);
    sigaddset(&nmask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &nmask, &omask);
#else
    //signal(SIGCHLD, SIG_IGN);
#endif

    pid = fork();
    if (pid == 0) {
        /* child */
        printf("child process(%d) started ...\n", getpid());
        sleep(15);
        printf("child process(%d) exit...\n", getpid());
    } else if (pid > 0) {
        printf("parent will wait for pid(%d)\n", pid);
        /* parent */
        pid_wait = waitpid(pid, &wstatus, 0);
        if (pid_wait > 0) {
            printf("wait child process %d successfully\n", pid_wait);
        } else if (pid_wait == -1) {
            perror("waitpid:");
            //printf("waitpid error for pid(%d) with error %s\n", pid, strerror(errno));
        }
        printf("pid_wait = %d\n", pid_wait);
//        pid_t waitpid(pid_t pid, int *wstatus, int options);
    } else {
        printf("fail to fork\n");
        return -1;
    }

    return 0;
}
