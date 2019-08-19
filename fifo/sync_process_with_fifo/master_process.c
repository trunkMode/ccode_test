/**
 *@func: sync status between process by fifo
 *@SP_MASTER_FIFO: common fifo to read by master and write by all other process
 *@note: master process will write msg to all other process by independent fifo defined in each process.
 *       all other process read each independent fifo, complete some task and then write back fifo index to master process.
 *       master process will check if all process has sent response to master, if yes, exit reboot, if no, keep select master_fifo or timeout.
 */
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#define SP_MASTER_FIFO "/tmp/master_fifo"
#define SP_MON_DAEMON_FIFO "/tmp/sp_mon_daemon_fifo"
#define SP_MON_DAEMON_FIFO1 "/tmp/sp_mon_daemon_fifo1"

#define RES_FIFO_NUM(a)    (sizeof(a)/sizeof(typeof(a[0])))

typedef enum {
    SP_MON_DAEMON_FIFO_INDEX = 0,
    SP_MON_DAEMON_FIFO1_INDEX = 0,
    __FIFO_INDEX_LAST,
    FIFO_INDEX_MAX = __FIFO_INDEX_LAST - 1,
}FIFO_INDEX_ENUM;


void sig_pipe_handler(int sig)
{
    return ;
}


int main()
{
    char *res_fifo[] = {
        SP_MON_DAEMON_FIFO,
        SP_MON_DAEMON_FIFO1,
    };
    char fifo_index[32];
    int sync_bit = 0, res_sync_bit = 0;
	int ret,fd[RES_FIFO_NUM(res_fifo)], master_fd, maxfd = 0;
    int i;
    struct sigaction act;
    act.sa_handler = sig_pipe_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPIPE, &act, NULL);

	char buf[1] = {0};
	fd_set rdset, rdset_save;
	struct timeval tv;

    memset(fd, sizeof(fd), 0x00);
	unlink(SP_MASTER_FIFO);

    printf("\narray size = %d\n", RES_FIFO_NUM(res_fifo));

	if (mkfifo(SP_MASTER_FIFO, 0777) < 0)
		printf("\nfailed to create fifo\n");

	master_fd = open(SP_MASTER_FIFO, O_RDWR);
	if (master_fd < 0) {
        perror("Failed:");
		printf("\nFailed to open fifo----\n\n");
    }

    /*write message*/
    for (i = 0; i < RES_FIFO_NUM(res_fifo); i++) {
        if (res_fifo[i] && access(res_fifo[i], F_OK) == 0) {
            printf("res_fifo[%d] = %s\n", i, res_fifo[i]);
            if ((fd[i] = open(res_fifo[i], O_WRONLY)) < 0) {
                fd[i] = 0;
            } else {
                sync_bit |= 1 << i;
                /*write message to process*/
                printf("write message to process to res_fifo[%d] = %s\n", i, res_fifo[i]); 
                ret = write(fd[i], buf, 1);
                if (ret != 1) 
                    printf("Failed to write msg to process\n");
                else 
                    sync_bit |= 1 << i;
            }
        } else {
            printf("res_fifo[%d] = %s don't exist\n", res_fifo[i]);
        }
    }
    printf("sync_bit = 0x%x\n", sync_bit);

    /*read response*/
#if 0
    FD_ZERO(&rdset_save);
 
    for (i = 0; i < RES_FIFO_NUM; i++) {
        if (sync_bit & (1 << i)) {
            FD_SET(fd[i], &rdset_save);
            if (fd[i] > maxfd)
                maxfd = fd[i];
        }
    } 
#endif   

    while (1) {
		tv.tv_usec = 0;
		tv.tv_sec = 10 * RES_FIFO_NUM(res_fifo);
        FD_ZERO(&rdset);
        FD_SET(master_fd, &rdset);
        maxfd = master_fd;
//        rdset = rdset_save;

		ret = select(maxfd + 1, &rdset, NULL, NULL, &tv);
		if (ret < 0) {
			printf("%s %d select() error: %s\n", __func__, __LINE__);
			perror("-----");
		} else if (ret == 0) {
            printf("%s timeout to read response from other process\n", __func__);
            break;
        } else {
             printf("%s some fd need read, ret = %d\n", __func__, ret); 
             if (FD_ISSET(master_fd, &rdset)) {
                memset(fifo_index, -1, sizeof(fifo_index));
                if ((ret = read(master_fd, fifo_index, sizeof(fifo_index))) > 0) {
                    printf("ret = %d\n", ret);
                    for (i = 0; i < ret; i++) {
                        printf("fifo_index[%d] = %d\n", i, fifo_index[i]);
                        if (fifo_index[i] >= 0)
                            res_sync_bit |= 1 << fifo_index[i];
                    }
                }
            }
		}
        printf("res_sync_bit = %d\n", res_sync_bit);
        /*check if all response has been received*/
        if (sync_bit == res_sync_bit) {
            printf("all response has been received\n");
            break;
        }

    }
    return 0;
		

	
}
