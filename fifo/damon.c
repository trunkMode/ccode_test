#include <unistd.h>
#include <fcntl.h>
#define FIFO_PATH "/tmp/my_fifo"
int main()
{
	int ret,fd;
	int buf[11] = {0};

//	if (mkfifo(FIFO_PATH, 0666) < 0)
//		printf("failed to create fifo\n");

	fd = open(FIFO_PATH, O_WRONLY);

	if (ret < 0)
		printf("Failed to open fifo\n");
printf("open fifo is ok\n");

	while (1)
	{
		memset(buf, 0x11, sizeof(buf));
		
		//ret = write(fd, buf, sizeof(buf));
		
	//	printf ("ret of write = %d\n", ret);

		sleep(1);
	}
printf("open fifo close\n");
	//fclose(fd);	
}
