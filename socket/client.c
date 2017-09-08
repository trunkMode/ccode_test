/*
 * @ socket client test program.
 * @ UDP client:
 *   	socket() --> [connect() associate server addr ] --> sendto()
 *      
 *
*/
#include <stdio.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <syslog.h>  
#include <errno.h>  
#include <stdlib.h>  
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define MAX_LISTEN_NUM 5  
#define SEND_BUF_SIZE 1024*1024*5
#define RECV_BUF_SIZE 1024*1024*1
#define SERVER_PORT 44433 


static void SignalHandler(int nSigno)  
{  
    signal(nSigno, SignalHandler);  
    switch(nSigno)  {  
        case SIGPIPE:  
            printf("Process will not exit\n");  
            break;  
        default:  
            printf("%d signal unregister\n", nSigno);  
            break;  
    }  
}  
  
static void InitSignalHandler()  
{  
    signal(SIGPIPE , &SignalHandler);  
} 



int main(int argc, char **argv)  
{
    int skfd, ret;
    time_t ts1, ts2;
	char recvbuf[RECV_BUF_SIZE] = {0}, sendbuf[SEND_BUF_SIZE] = {0};

	int recvlen = 0, retlen = 0, sendlen = 0, leftlen = 0;
	char *ptr = NULL;
	struct sockaddr_in server_addr;
    
    InitSignalHandler();

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	//inet_aton("10.103.12.202", (struct in_addr *)&server_addr.sin_addr);
	inet_aton("10.103.12.202", (struct in_addr *)&server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

	/*create socket*/
    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
        printf("##@client@ create socket failed");
        exit(1);  
    }
     
    ts1 = time(NULL);
	/*connecting to server '10.103.12.202'*/
    if((ret = connect(skfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {  
		printf("##error = %d\n", errno);
        printf("\n@client@ connect socket failed@, ret = %d\n", ret);
		perror("error :");
		ts2 = time(NULL);
		printf("time interval is ts2 - ts1 = %d\n", ts2 - ts1);
        exit(1);
    } 
#if 0
    int cnt = 1;
    while (cnt--) {
        char a = 0x11;
        retlen = write(skfd, recvbuf, sizeof(recvbuf));
        printf("retlen = %d, sizeof(recvBuf) = %d\n", retlen, sizeof(recvbuf));
    }
    return 0;
#endif
    int b_on = 1;
    int maxfds = skfd + 1;
    fd_set rdset;
    struct timeval tv;
    
    FD_ZERO(&rdset);
    FD_SET(skfd, &rdset);
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    
    ioctl(skfd, FIONBIO, &b_on);
    
    while (1) {
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        ret = select(maxfds, &rdset, NULL, NULL, &tv);
        
        if (ret < 0) {
            perror("select error :");
            continue;
        } else if (ret == 0) {
            printf("select timeout\n");
            continue;
        } else {
            if (FD_ISSET(skfd, &rdset)) {
                while ((retlen = read(skfd, recvbuf, sizeof(recvbuf))) > 0) {
                    printf("[client] read data len is %d\n", retlen);
                }
                if (retlen < 0) {
                    if (errno == EWOULDBLOCK) {
                        printf("[client] there is no data to read!\n");
                        continue;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    return 0;
#if 0
     //receive data  
     recvlen = 0;  
     retlen = 0;  
     ptr = recvbuf;  
     leftlen = RECV_BUF_SIZE -1 ;  
     //do  
     {  
         retlen = recv(skfd, ptr, leftlen, 0) ; 
        printf("retlen = %d\n", retlen); 
      if(retlen < 0)  
      {  
          if(errno == EINTR)  
            retlen = 0;  
        else  
            exit(1);  
      }  
      recvlen += retlen;  
      leftlen -= retlen;  
      ptr += retlen;  
     }  
     //while(recvlen && leftlen);  

     sprintf(sendbuf, "hello server/n");  
#endif
     //send data
	printf("@client@ start to send data to server after 5s ..@@@@@@@@@@@@@@@@@.\n");
//	sleep(1);
	while (1) {
	memset(sendbuf, 0x00, sizeof(sendbuf));
    memset(sendbuf, 0xaa, sizeof(sendbuf) - 1);
	sendlen = sizeof(sendbuf);
	retlen = 0;
	leftlen = sendlen;
	ptr = sendbuf;
	int test = 3;
	int total_len = 0;
//sleep(1);
	while(leftlen) {
	//	printf("@client@ sending a first string...\n");
		///retlen = send(skfd, ptr, sendlen, 0);
        retlen = write(skfd, ptr, sendlen);
	///	printf("@client@ sending completed....retlen= %d.......@@@@@@@@@@@@@@@@@@\n", retlen);
// 		sleep(1);
		if(retlen < 0) {
			perror("@client@ send error:");
			printf("@client@ errno = %d\n", errno);
			if(errno == EINTR)
				retlen = 0;
			else
				exit(1);
		} else if (retlen == 0) {
			printf("@client@  connection has been closed by server\n");
			exit(0);
		}
		total_len += retlen;
		leftlen -= retlen;
		//ptr += retlen;
	}
	}
	//printf("@client@ connection will be closed in 1s....\n");
	sleep(5);
	printf ("@client@ send...\n");
	close(skfd);
}  
/*hgr update*/
