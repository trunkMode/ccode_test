#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef void*(*FUNCPTR)(void *);

void *thread_task(void *arg)
{
	int ret;
	printf("thread task has been started !\n");

	ret = pthread_detach(pthread_self());
	printf("(1) call pthread_detach return %d\n", ret);

	ret = pthread_detach(pthread_self());
	printf("(2) call pthread_detach return %d\n", ret);

	ret = pthread_detach(pthread_self());
	printf("(3) call pthread_detach return %d\n", ret);
	return NULL;
}
int main()
{
	pthread_t tid;

	if (pthread_create(&tid, NULL, (FUNCPTR)thread_task, NULL)) {
		printf("Failed to create threads\n");
		return -1;
	}

	sleep(20);

	return 0;
}
