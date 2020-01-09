/**
 *@ func test if pthread_cond_signal will be missed when no one is listen for this signal 
 */
#include <pthread.h>
#include <unistd.h>

typedef void *(*FUNC_PTR)(void *arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void thread_cond_wait(void *arg)
{
    printf("0x%x\n", (int)pthread_self());
    pthread_detach(pthread_self());
//    sleep(5);
#if 1
    pthread_mutex_lock(&mutex);
    while (1) {
        printf("------------------------\n");
        printf("%s: start to wait cond!\n", __func__);
        pthread_cond_wait(&cond, &mutex);

        printf("%s: got a cond ready and do 5s task !\n\n", __func__);
        sleep(2);
        printf("%s: finish 5s task\n", __func__);
    }
#endif
}

void thread_cond_signal(void *arg)
{
    int i = 0;
    pthread_detach(pthread_self());
    sleep(2);
#if 1
    while (i++ < 10) {
        printf("****************\n");
        printf("%s: start to signal cond ready in the next 1s \n", __func__);
        sleep(1);

        //pthread_mutex_lock(&mutex);
        printf("%s: send cond signal\n", __func__);
        pthread_cond_signal(&cond);
        printf("%s: send cond signal successfully\n", __func__);
        //pthread_mutex_unlock(&mutex);
    }
#endif
}

int main()
{
    pthread_t tid_cond_wait, tid_cond_signal;
    
    if (pthread_create(&tid_cond_wait, NULL, (FUNC_PTR) thread_cond_wait, NULL)) {
        printf("Failed to create a thead !\n");
    }
    
    if (pthread_create(&tid_cond_signal, NULL, (FUNC_PTR) thread_cond_signal, NULL)) {
        printf("Failed to create a thead !\n");
    }
    
    sleep(1000);
    //pthread_join(tid_cond_wait, NULL);
    //pthread_join(tid_cond_signal, NULL);
    return 0;
}
