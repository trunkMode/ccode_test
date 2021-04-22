#include <pthread.h>


int main()
{
	int type = 0;
	pthread_mutexattr_t mutex_attr;

	if (pthread_mutexattr_init(&mutex_attr)) {
		printf("Failed to init mutex attr\n");
		return -1;
	}

	if (pthread_mutexattr_gettype(&mutex_attr, &type)) {
		printf("failed to get mutex attr type\n");
		return -1;
	}
	printf("type = 0x%x\n", type);
}
