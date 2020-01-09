#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int debug_rwlock = 1;
pthread_rwlock_t test_rwlock;

//    pthread_rwlockattr_setkind_np(&test_rwlock, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);  
#define PTHREAD_RWLOCK_INIT()                       \
{                                                   \
    if (pthread_rwlock_init(&test_rwlock, NULL)) {        \
        printf("Failed to init  test_rwlock.\n");   \
        return -1;                                  \
    }                                               \
    pthread_rwlockattr_setkind_np(&test_rwlock, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP); \
    int kind = 0;                                       \
    pthread_rwlockattr_getkind_np(&test_rwlock, &kind); \
    printf("kind  = %d \n", kind);                      \
}

#define PTHREAD_RWLOCK_RDLOCK()                                         \
{                                                                       \
    printf("***%s %d: trying to acquire rdlock\n",                       \
            __func__, __LINE__);                                        \
    int ret = pthread_rwlock_rdlock(&test_rwlock);                      \
    if (debug_rwlock) {                                                 \
        if (ret) {                                                      \
            printf("%s %d: rwlock: acquire rdlock failed (%d)\n\n",     \
                    __func__, __LINE__, ret);                           \
        } else {                                                        \
            printf("%s %d: rwlock acqurire rdlock successfully.\n\n",   \
                    __func__, __LINE__);                                \
        }                                                               \
    }                                                                   \
}

#define PTHREAD_RWLOCK_WRLOCK()                                         \
{                                                                       \
    printf("---%s %d: trying to acquire rwlock\n",                       \
            __func__, __LINE__);                                        \
    int ret = pthread_rwlock_wrlock(&test_rwlock);                      \
    if (debug_rwlock) {                                                 \
        if (ret) {                                                      \
            printf("%s %d: rwlock: acquire rdlock failed (%d)\n\n",     \
                    __func__, __LINE__, ret);                           \
        } else {                                                        \
            printf("%s %d: rwlock: acquire wrlock successfully.\n\n",   \
                    __func__, __LINE__);                                \
        }                                                               \
    }                                                                   \
}

#define PTHREAD_RWLOCK_UNLOCK(is_read_lock)                             \
{                                                                       \
    const char *type = is_read_lock ? "rdlock" : "wrlock";              \
    printf("%s %d: trying to unlock %s\n\n",                            \
            __func__, __LINE__, is_read_lock ? "rdlock" : "wrlock");    \
    int ret = pthread_rwlock_unlock(&test_rwlock);                      \
    if (debug_rwlock) {                                                 \
        if (ret) {                                                      \
            printf("%s %d: rwlock: unlock %s failed (%d)\n\n",            \
                    __func__, __LINE__, type, ret);                     \
        } else {                                                        \
            printf("%s %d: rwlock: unlock %s successfully.\n\n",          \
                    __func__, __LINE__, type);                          \
        }                                                               \
    }                                                                   \
}

void *thread_read_a(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        sleep(2);
        usleep(1000*200);
        PTHREAD_RWLOCK_RDLOCK();
        sleep(2);
        PTHREAD_RWLOCK_UNLOCK(1);
    }
    return NULL;
}

void *thread_read_b(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        sleep(2);
        usleep(1000*400);
        PTHREAD_RWLOCK_RDLOCK();
        sleep(1);
        PTHREAD_RWLOCK_UNLOCK(1);
    }
    return NULL;
}

void *thread_write_c(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        PTHREAD_RWLOCK_WRLOCK();
        sleep(1);
        PTHREAD_RWLOCK_UNLOCK(0);
    }

    return NULL;
}

int main()
{
    pthread_t tid;

    PTHREAD_RWLOCK_INIT();
    if (pthread_create(&tid, NULL, thread_read_a, NULL)) {
        printf("Failed to create thread A\n");
        return -1;
    }

    if (pthread_create(&tid, NULL, thread_read_b, NULL)) {
        printf("Failed to create thread B\n");
        return -1;
    }

    if (pthread_create(&tid, NULL, thread_write_c, NULL)) {
        printf("Failed to create thread C\n");
        return -1;
    }

    sleep(30);
}
