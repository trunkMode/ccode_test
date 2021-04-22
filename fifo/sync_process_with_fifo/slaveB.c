#include <unistd.h>
#include <fcntl.h>
#define SP_MASTER_FIFO "/tmp/master_fifo"
#define SP_MON_DAEMON_FIFO1 "/tmp/sp_mon_daemon_fifo1"

int main()
{

	int ret,fd, master_fd;
    int i;

	char buf[1] = {0};
	fd_set rdset;
	struct timeval tv;

    memset(fd, sizeof(fd), 0x00);
	unlink(SP_MON_DAEMON_FIFO);

	if (mkfifo(SP_MON_DAEMON_FIFO , 0777) < 0) {
        perror("Failed::");
		printf("failed to create fifo\n");
    }

	fd = open(SP_MON_DAEMON_FIFO, O_RDONLY);
	if (fd < 0)
		printf("Failed to open fifo"SP_MON_DAEMON_FIFO"\n");

    sleep(5);

    printf("open master fifo\n");
	master_fd = open(SP_MASTER_FIFO, O_WRONLY);
	if (master_fd  < 0)
		printf("Failed to open fifo "SP_MASTER_FIFO"\n");

    printf("read spmondaemone fifo\n");
    ret = read(fd, buf, sizeof(buf));

    if (ret > 0) {
        printf("receive rebooting info, saving files\n");
        printf("write response\n");
        ret = write(master_fd, buf, sizeof(buf));
        ret = write(master_fd, buf, sizeof(buf));
    }
    printf("exit ret = 0\n");
    close(fd);
    close(master_fd);
    return 0;
	
}
