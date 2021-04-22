#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>

pthread_mutex_t         mutex;
pthread_mutexattr_t     mutex_attr;
pthread_cond_t          cond;
#define COND_WAIT_COUNT     1

#define PTHREAD_TIMED_WAIT(task_name, n)                                            \
do {                                                                                \
    int ret;                                                                        \
    struct timespec ts;                                                             \
    struct timeval tv;                                                              \
    gettimeofday(&tv, NULL);                                                        \
    ts.tv_sec = tv.tv_sec + n;                                                      \
    ts.tv_nsec = tv.tv_usec *1000;                                                  \
    ret = pthread_cond_timedwait(&cond, &mutex, &ts);                               \
    printf("%s %d: ret = %d\n", name, __LINE__, ret);                           \
    if (!ret) {                                                                     \
        printf("%s %d: cond timed wait return 0\n", name, __LINE__);            \
    } else {                                                                        \
        if (ETIMEDOUT == ret) {                                                     \
            printf("%s %d: cond timed wait is timedout\n", name, __LINE__);     \
        } else {                                                                    \
            printf("%s %d: cond timed wait error\n", name, __LINE__);           \
        }                                                                           \
    }                                                                               \
} while (0)

void *cond_wait_thread(void *arg)
{
    int i = (int)arg;
    char name[16];
    pthread_detach(pthread_self());

    snprintf(name, sizeof(name), "cond_wait_%d", i);
    prctl(PR_SET_NAME, name, 0, 0, 0);

    while (1) {
        pthread_mutex_lock(&mutex);
        PTHREAD_TIMED_WAIT(name, 2);
        printf("%s %d: %s is processing something\n", name, __LINE__, name);
        pthread_mutex_unlock(&mutex);
    //    sleep(1);
    }
}

void *cond_notify_thread(void *arg)
{
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, "notify", 0, 0, 0);

    while (1) {
        sleep(4);
        pthread_mutex_lock(&mutex);
#if 1
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond);
#else
        pthread_cond_broadcast(&cond);
#endif
        printf(" =============== notify end============ \n");
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    int i;
    pthread_t tid;

    /* init */
    if (pthread_cond_init(&cond, NULL) || pthread_mutexattr_init(&mutex_attr) ||
        pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP) ||
        pthread_mutex_init(&mutex, &mutex_attr)) {
        printf("pthread mutex init faile \n");
        return -1;
    }

    for (i = 0; i < COND_WAIT_COUNT; i++) {
        if (pthread_create(&tid, NULL, cond_wait_thread, (void *)i)) {
            printf("Failed to create cond_wait_thread_%d\n", i);
            return -1;
        }
    }
#if 1
    if (pthread_create(&tid, NULL, cond_notify_thread, NULL)) {
        printf("Failed to create cond_notify_thread \n");
        return -1;
    }
#endif
    sleep(100);
    /*destroy when exit */
    pthread_mutexattr_destroy(&mutex_attr);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
