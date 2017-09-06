#include <unistd.h>
#include "sem_sync.h"
#include <semaphore.h>
//#include <time.h>
//#include <sys/time.h>

int main()
{
    sem_t *sem;
    struct timespec tv;
    int val;

    if ((sem = sem_open(SEM_NAME, O_CREAT, 0600, 0)) == SEM_FAILED) {
        perror("unable to create semaphore");
        sem_unlink(SEM_NAME);
        return -1;
    }

    tv.tv_sec = time(NULL) + 10;
    tv.tv_nsec = 0;
    printf("tv.tv_sec = %d\n", tv.tv_sec);
    sleep(10);
    
    int proc_cnt = 2;   
    /*while (1)*/ {
        printf("\n[process A] take sem and timeout in 5s...\n");
        system("killall -SIGUSR1 processB");
        system("killall -SIGUSR1 processC");

        while (proc_cnt--) {
            if (sem_timedwait(sem, &tv) == 0) {
                printf("tv.tv_sec = %d\n", tv.tv_sec);
                printf("\n [process A] takes the sem successfully\n");
            } else {
                perror("\nsem_timedwait failed:");
                if (sem_getvalue(sem, &val)) {
                    perror("\n[process A] Failed to get sem value:");    
                } else {
                    printf("\n[process A] value = %d\n", val);
                }
                break;
            }
        }
    }
    sem_close(sem);
    sem_unlink(SEM_NAME);
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

