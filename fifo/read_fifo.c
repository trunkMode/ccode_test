#include <unistd.h>
#include <fcntl.h>
#define FIFO_PATH "/tmp/my_fifo"
int main()
{
	int ret,fd;
	int buf[32] = {0};
	fd_set rdset;
	struct timeval tv;
	
	unlink(FIFO_PATH);

	if (mkfifo(FIFO_PATH, 0777) < 0)
		printf("failed to create fifo\n");
//	fd = open(FIFO_PATH, O_RDWR|O_NONBLOCK);//O_NONBLOC
	fd = open(FIFO_PATH, O_RDWR);
	if (ret < 0) {
		printf("Failed to open fifo\n");
	}
		
    printf("open fifo is ok in read \n");

	while (1)
	{
#if 1
		tv.tv_usec = 0;
		tv.tv_sec = 60;
		FD_ZERO(&rdset);
		FD_SET(fd, &rdset);
		ret = select(fd + 1, &rdset, NULL, NULL, &tv);
		if (ret == -1) {
			printf("%s %d select() error: %s\n", __func__, __LINE__);
			perror("-----");
		} else if (ret) {
			ret = read(fd, buf, sizeof(buf));
		
			printf ("ret of read = %d\n", ret);
		}
#endif
#if 0

	//	system("./write");
		ret = read(fd, buf, sizeof(buf));
		
		printf ("ret of read = %d\n", ret);
		printf("buf= %s \n", buf);

		sleep(1);
#endif
		
	}
	
}
