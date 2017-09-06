/*
 * @function: sp_master synchronization using semaphore and signal
 *      sp tell the reboot status to spMonDaemon and spMonDaemon save files and release the sem to reboot
 * @note: 1) bug1--
        we need set reboot flag, or else the threadB may not release the semophore and processA will wait semaphore to be timeout.
    
        
*/
//#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>

#define SEM_SYNC                    "/SEM_SYNC"

pthread_t tid_user;

pid_t pid;

/*
       int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);

*/
int reboot = 0;
int send_to_thread = 1;
void sig_usr1_handler(int signo)
{
    printf("[process B] receive signal\n");
    if (reboot == 0) {
        printf("send siguser1 to theadB\n");
    
        //pthread_kill(tid_user, SIGUSR1);
        reboot = 1;
    }
}

void sig_usr2_handler(int signo)
{
    printf("[sig user2] xxxxx\n");
}
void *threadC(void *arg)
{
    int ret;
    int val;
    sem_t *sem;
    prctl(PR_SET_NAME, "theadC", NULL, NULL, NULL);

#if 0
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sig_usr1_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);
#endif

    //sleep(100);
    sleep(1);
    while(1) {
       if ((sem = sem_open(SEM_SYNC, O_CREAT, 0600, 0)) == SEM_FAILED) {
          perror("unable to create semaphore");
            sem_unlink(SEM_SYNC);
            return -1;
        }
        printf("\n[%s] wants to reboot in 5 s\n", __func__);
        printf("\n[%s] sleeping ...\n", __func__);
        if (!reboot) 
            ret = sleep(1);
        printf("[%s] sleep has left %d secs\n", __func__, ret);
        
        printf("[%s] save files....\n", __func__);
        
        if (1) {
            reboot = 0;
            if (sem_post(sem) == 0) {
                printf("\n [%s] release the sem successfully\n", __func__);
            } else {
                perror("\nrelease sem failed:");
                if (sem_getvalue(sem, &val)) {
                    perror("\n[thread C] Failed to get sem value:");    
                } else {
                    printf("\n[%s] value = %d\n", __func__);
                }
            }
        } 
  
    sem_close(sem);
    sem_unlink(SEM_SYNC);
  
    printf("\n[%s] exit...\n", __func__);
    }
    return 0;    
}
int main()
{

#if 1
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sig_usr1_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);
#endif

    
    if (pthread_create(&tid_user, NULL, threadC, NULL) != 0) {
        printf("Failed to create threadB\b");
        return -1;
    }
    pthread_join(tid_user, NULL);
    printf("pthread_join break\n");

    return 0;
}

