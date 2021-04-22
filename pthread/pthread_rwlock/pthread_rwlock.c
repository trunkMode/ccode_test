#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int debug_rwlock = 1;
pthread_rwlock_t test_rwlock;
pthread_rwlockattr_t test_rwlock_attr;

//    pthread_rwlockattr_setkind_np(&test_rwlock, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);  
#define PTHREAD_RWLOCK_INIT()                               \
{                                                           \
    if (pthread_rwlockattr_init(&test_rwlock_attr)) {       \
        printf("Failed to init test_rwlock_attr.\n");       \
        return -1;                                          \
    }                                                       \
    if (pthread_rwlockattr_setkind_np(&test_rwlock_attr,    \
        PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP)) {    \
        printf("Failed to set rwlock attr kind.\n");        \
        return -1;                                          \
    }                                                       \
    if (pthread_rwlock_init(&test_rwlock,                   \
                &test_rwlock_attr)) {                       \
        printf("Failed to init  test_rwlock.\n");           \
        return -1;                                          \
    }                                                       \
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
            printf("%s %d: rwlock: acquire wrlock failed (%d)\n\n",     \
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
        PTHREAD_RWLOCK_RDLOCK();
        printf("%s %d: take rd lock 1\n", __func__, __LINE__);
        PTHREAD_RWLOCK_RDLOCK();
        printf("%s %d: take rd lock 2\n", __func__, __LINE__);
        PTHREAD_RWLOCK_RDLOCK();
        printf("%s %d: take rd lock 3\n", __func__, __LINE__);
        sleep(5);
        printf("%s %d: give rd lock 3\n", __func__, __LINE__);
        PTHREAD_RWLOCK_UNLOCK(1);
        printf("%s %d: give rd lock 2\n", __func__, __LINE__);
        PTHREAD_RWLOCK_UNLOCK(1);
        printf("%s %d: give rd lock 1\n", __func__, __LINE__);
        PTHREAD_RWLOCK_UNLOCK(1);
    }
    return NULL;
}

void *thread_read_b(void *arg)
{
    pthread_detach(pthread_self());

    while (1) {
        sleep(1);
        PTHREAD_RWLOCK_RDLOCK();
        PTHREAD_RWLOCK_RDLOCK();
        sleep(1);
        PTHREAD_RWLOCK_UNLOCK(1);
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
#if 1
    if (pthread_create(&tid, NULL, thread_write_c, NULL)) {
        printf("Failed to create thread C\n");
        return -1;
    }
#endif
    sleep(30);
}
