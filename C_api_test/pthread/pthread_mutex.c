#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int debug_lock = 1;
pthread_mutex_t test_lock;
pthread_mutexattr_t test_lock_attr;

#define PTHREAD_MUTEX_INIT()                                \
{                                                           \
    const char *__str = NULL;                               \
    do {                                                    \
        const char *__str = NULL;                           \
        if (pthread_mutexattr_init(&test_lock_attr)) {      \
            __str = "Failed to init lock attr.";            \
            break;                                          \
        }                                                   \
        if (pthread_mutexattr_settype(&test_lock_attr,      \
                PTHREAD_MUTEX_RECURSIVE_NP)) {              \
            __str = "Failed to set type for lock attr.";    \
            break;                                          \
        }                                                   \
        if (pthread_mutex_init(&test_lock, &test_lock_attr)) {         \
            __str = "Failed to init test lock.";            \
            break;                                          \
        }                                                   \
    } while(0);                                             \
    if (__str) {                                            \
        printf("%s\n", __str);                              \
    }                                                       \
}

#define PTHREAD_MUTEX_LOCK()                                            \
{                                                                       \
    printf("***%s %d: trying to acquire lock\n",                        \
            __func__, __LINE__);                                        \
    int ret = pthread_mutex_lock(&test_lock);                           \
    if (debug_lock) {                                                   \
        if (ret) {                                                      \
            printf("%s %d: lock: acquire lock failed (%d)\n\n",         \
                    __func__, __LINE__, ret);                           \
        } else {                                                        \
            printf("%s %d: lock acqurire lock successfully.\n\n",       \
                    __func__, __LINE__);                                \
        }                                                               \
    }                                                                   \
}

#define PTHREAD_MUTEX_UNLOCK()                                          \
{                                                                       \
    int ret = pthread_mutex_unlock(&test_lock);                         \
    if (debug_lock) {                                                   \
        if (ret) {                                                      \
            printf("%s %d: lock: unlock failed (%d)\n\n",               \
                    __func__, __LINE__, ret);                           \
        } else {                                                        \
            printf("%s %d: lock: unlock successfully.\n\n",             \
                    __func__, __LINE__);                                \
        }                                                               \
    }                                                                   \
}

void *thread_read_a(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        PTHREAD_MUTEX_LOCK();
        printf("%s %d: mutex lock 1\n", __func__, __LINE__);
        PTHREAD_MUTEX_LOCK();
        printf("%s %d: mutex lock 2\n", __func__, __LINE__);
        sleep(2);
        PTHREAD_MUTEX_UNLOCK();
        printf("%s %d: mutex unlock 2\n", __func__, __LINE__);
        PTHREAD_MUTEX_UNLOCK();
        printf("%s %d: mutex unlock 1\n", __func__, __LINE__);
    }
    return NULL;
}

void *thread_read_b(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
//        usleep(1000*10);
        PTHREAD_MUTEX_LOCK();
        sleep(5);
        PTHREAD_MUTEX_UNLOCK();
    }
    return NULL;
}

void *thread_write_c(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        PTHREAD_MUTEX_LOCK();
        sleep(1);
        PTHREAD_MUTEX_UNLOCK();
    }

    return NULL;
}

int main()
{
    pthread_t tid;

    PTHREAD_MUTEX_INIT();
    if (pthread_create(&tid, NULL, thread_read_a, NULL)) {
        printf("Failed to create thread A\n");
        return -1;
    }

    if (pthread_create(&tid, NULL, thread_read_b, NULL)) {
        printf("Failed to create thread B\n");
        return -1;
    }
#if 0
    if (pthread_create(&tid, NULL, thread_write_c, NULL)) {
        printf("Failed to create thread C\n");
        return -1;
    }
#endif

    sleep(30);
}
