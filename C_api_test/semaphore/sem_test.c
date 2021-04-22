/**
 *@ func test if pthread_cond_signal will be missed when no one is listen for this signal 
 */
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef void *(*FUNC_PTR)(void *arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
sem_t sem_test;

void *sem_wait_thread(void *arg)
{
    char *p = "hello world";
    printf("%s started\n", __func__);
    while (1) {
        printf("------------take sem------------\n");
        sem_wait(&sem_test);
        printf("%s: got a sem!\n", __func__);
        sleep(5);
        printf("------------------------\n");
    }
}

void *sem_post_thread(void *arg)
{
    int i = 0;
    printf("%s started\n", __func__);
    while (i++ < 10) {
        printf("*******give sem*********\n");
        sem_post(&sem_test);
        sleep(1);
        printf("**********************\n");
    }
}

int main()
{
    pthread_t tid_sem_wait, tid_sem_post;
    
    if (pthread_create(&tid_sem_wait, NULL, (FUNC_PTR) sem_wait_thread, NULL)) {
        printf("Failed to create a thead !\n");
    }
    
    if (pthread_create(&tid_sem_post, NULL, (FUNC_PTR)sem_post_thread, NULL)) {
        printf("Failed to create a thead !\n");
    }
    
    pthread_join(tid_sem_wait, NULL);
    pthread_join(tid_sem_post, NULL);
    return 0;
}
