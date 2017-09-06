/*
 * @function: processA and processB synchronization using semaphore and signal
 * @note: 1) bug1--
        we need set reboot flag, or else the threadB may not release the semophore and processA will wait semaphore to be timeout.
        
*/
//#include <unistd.h>
#include "sem_sync.h"
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>

pthread_t tid_user;
sem_t *sem;
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
    if (send_to_thread) {
        printf("send siguser1 to theadB\n");
        pthread_kill(tid_user, SIGUSR1);
        reboot = 1;
        send_to_thread = 0;
    }
}

void sig_usr2_handler(int signo)
{
    printf("[sig user2] xxxxx\n");
}
void *threadB(void *arg)
{
    int ret;
    int val;
    prctl(PR_SET_NAME, "theadB", NULL, NULL, NULL);
//    pthread_detach(tid_user);

#if 1
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sig_usr1_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);
#endif

    sleep(10);
    /*while (1)*/ {
        printf("\n[thread B] wants to reboot in 5 s\n");
        printf("\n[thread B] sleeping ...\n");
        if (!reboot) 
            ret = sleep(5);
        printf("[thread B] sleep has left %d secs\n", ret);
        
//        printf("[thread B] sleep again\n");
//        sleep(10);
        printf("[thread B] save files....\n");
        
        if (reboot) {
            reboot = 0;
            if (sem_post(sem) == 0) {
                printf("\n [thread] release the sem successfully\n");
            } else {
                perror("\nrelease sem failed:");
                if (sem_getvalue(sem, &val)) {
                    perror("\n[thread B] Failed to get sem value:");    
                } else {
                    printf("\n[thread B] value = %d\n");
                }
            }
        } 
    }
    sem_close(sem);
    sem_unlink(SEM_NAME);
    printf("\n[thread B] exit...\n");
    return 0;    
}
int main()
{

//    struct timesepc tv;
    int val, ret;
#if 0
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sig_usr1_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);
#endif

    if ((sem = sem_open(SEM_NAME, O_CREAT, 0600, 0)) == SEM_FAILED) {
        perror("unable to create semaphore");
        sem_unlink(SEM_NAME);
        return -1;
    }
    
    if (pthread_create(&tid_user, NULL, threadB, NULL) != 0) {
        printf("Failed to create threadB\b");
        return -1;
    }
    pthread_join(tid_user, NULL);
    printf("pthread_join break\n");

#if 0   
#if 0
    tv.tv_sec = time(NULL) + 10;
    tv.tv_nsec = 0;
    sleep(20);
#endif
    /*while (1)*/ {
        printf("\n[process B] wants to reboot in 5 s\n");
        printf("\nprocess B] sleeping ...\n");
        ret = sleep(60);
        printf("[process B] sleep has left %d secs\n", ret);
        
        printf("[process B] sleep again\n");
        sleep(10);
        printf("[process B] save files\n");
        
        if (sem_post(sem) == 0) {
            printf("\n [processB] release the sem successfully\n");
        } else {
            perror("\nrelease sem failed:");
            if (sem_getvalue(sem, &val)) {
                perror("\n[process B] Failed to get sem value:");    
            } else {
                printf("\n[process B] value = %d\n");
            }
        }
    }
    sem_close(sem);
    sem_unlink(SEM_NAME);
    return 0;
#endif;
    return 0;
}
#if 0

        while (1)
        {
            printf("\nsending signal to other processes\n");
            printf("\ntake sem\n");
            sem_wait
//            getchar();

            sem_getvalue(sem_a_task_is_free, &val);
            printf("The sem is %d\n", val);
            sem_post(sem_a_task_is_free);
            sem_getvalue(sem_a_task_is_free, &val);
            printf("The sem after sem post is %d\n", val);
            printf("send->a_task_is_free\n");
            
            printf("\nwaite sem_b_task_is_free\n");
            //sem_wait(sem_b_task_is_free);
            printf("recv<-sem_b_task_is_free\n");
        }

        sem_close(sem_a_task_is_free);
        sem_unlink(SEM_A_TASK_IS_FREE);

        _exit(0);
    }
#endif

