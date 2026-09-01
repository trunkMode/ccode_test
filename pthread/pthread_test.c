#include <stdio.h>
#include <pthread.h>
#include <assert.h>

volatile int a = 0;
volatile int b = 0;

void* writer(void* arg)
{
    a = 1;          // write #1
    b = 1;          // write #2 (may become visible first!)
    return NULL;
}

void* reader(void* arg)
{
    while (b == 0); // wait until writer sets b

    // NO MEMORY BARRIER HERE
    if (a == 0) {
        printf("❌ Reordering observed: b=1 but a=0\n");
        assert(0);
    }
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    for (int i = 0; i < 10000000; i++) {
        a = 0;
        b = 0;

        pthread_create(&t2, NULL, reader, NULL);
        pthread_create(&t1, NULL, writer, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
    }

    printf("Done\n");
    return 0;
}

