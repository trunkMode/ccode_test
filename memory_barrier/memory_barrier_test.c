#include <unistd.h>
#include <pthread.h>

typedef struct {
    int a;
    char b;
    int c;
}gptr_t;

gptr_t *gptr = NULL;
gptr_t data, data1;
void *task_a(void *arg)
{
    gptr_t *tmp = NULL;
    pthread_detach(pthread_self());

    while (1) {
        tmp = gptr;

        if (tmp) {
            printf("\na = %d, b = %d, c = %d\n", tmp->a, tmp->b, tmp->c);
            sleep(1);
        }
    }
}

void *task_b(void *arg)
{
    int i = 8,k = 6;
    pthread_detach(pthread_self());
    while (1) {
        data.a = i;
        data.b = i;
        data.c = i++;

        if (i > 100)
            i = 8;
        gptr = &data;

        sleep(1);
        data1.a = k;
        data1.b = k;
        data1.c = k++;
        if (k > 100) 
            k = 6;
        gptr = &data1;
    }
}

int main()
{
    pthread_t tid;

    if (pthread_create(&tid, NULL, task_a, NULL)) {
        printf("Failed to create task a\n");
        return -1;
    }


    if (pthread_create(&tid, NULL, task_b, NULL)) {
        printf("Failed to create task b\n");
        return -1;
    }


    sleep(100);
}
