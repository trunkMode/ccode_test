
#include <unistd.h>
#include <stdio.h>  
#include <string.h>
/*socket*/
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>  
#include <stdlib.h>  
#include <syslog.h>  
#include <errno.h>
#include <signal.h>
#define MAX_LISTEN_NUM 5  
#define SEND_BUF_SIZE 1024*1024 
#define RECV_BUF_SIZE 1024*1024*5
#define LISTEN_PORT 44433 
#define FFT_SOCKET_TIMEOUT  5

#define SERVER_DEBUG(FMAT,ARGS...) printf("[server]%s %d:"FMAT"\n", __func__, __LINE__, ## ARGS)
static void SignalHandler(int nSigno)  
{  
    signal(nSigno, SignalHandler);  
    switch(nSigno)  
    {  
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


int main()
{
    int max_fd = 0;
    int server_sock = 0;
    int app_sock = 0;
    int count = 0;
    struct sockaddr_in serveraddr, clientaddr;

    int socklen = sizeof(clientaddr);
    char sendbuf[SEND_BUF_SIZE] = {0};
    char recvbuf[RECV_BUF_SIZE] = {0};
    int sendlen = 0, recvlen = 0, retlen = 0, leftlen = 0;

    char *ptr = NULL;
    InitSignalHandler(); 
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset((void *)&clientaddr, 0, sizeof(clientaddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(LISTEN_PORT);
    serveraddr.sin_addr.s_addr = htons(INADDR_ANY);

    /*create a tcp socket for server to listen*/
    if((server_sock = socket(AF_INET, SOCK_STREAM  , 0)) < 0) //SOCK_NONBLOCK
    {
        syslog(LOG_ERR, "%s:%d, create socket failed", __FILE__, __LINE__);
        exit(1);
    }
    /* set sock options to reuse addr */
    int reuse_addr = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
        perror("Failed to set reuse addr:");
        exit(1);
    }
    
    /*bind server socket to addr 'INADD_ANY'*/
    if(bind(server_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf( "%s:%d, bind socket failed", __FILE__, __LINE__);
        exit(1);
    }
    /* listen on this socket*/
    if(listen(server_sock, MAX_LISTEN_NUM) < 0)
    {
        syslog(LOG_ERR, "%s:%d, listen failed", __FILE__, __LINE__);
        exit(1);
    }

//////////////////////////
//select socket fds to monitor if there is any fd to be read or written
///////////////////////////
    fd_set rd;
    struct timeval tv;

    while (1)
    {
        FD_ZERO(&rd);
        FD_SET(server_sock, &rd);

        tv.tv_usec = 0;
        tv.tv_sec = FFT_SOCKET_TIMEOUT;
        max_fd = server_sock + 1;
#if 0
	/********** using select ************/
	printf("#########\n");
        /*select on the server socket*/
        int n = select(max_fd, &rd, NULL, NULL, &tv);

        if (n < 0)
        {
            printf("failed in select in FFT socket\n");
            return 0;
        }   
        else if (n == 0)
        {
            //printf("FFT socket select timeout\n");
            continue;
        }
        else if (FD_ISSET(server_sock, &rd))
#endif
#if 1
	/*********** using poll *************/
	
#endif
        {
            SERVER_DEBUG("server started\n");
	        socklen = sizeof(struct sockaddr_in);
	        SERVER_DEBUG("struct sockaddr_in is %d\n", socklen);
            /*accept the connection from client*/
            app_sock = accept(server_sock, (struct sockaddr *)&clientaddr, &socklen);
            
            if (app_sock < 0)
            {
                //syslog(LOG_ERR, "%s:%d, accept failed", __FILE__, __LINE__);
                SERVER_DEBUG("accept one client failed\n");
                exit(1);
            }
#if 1
            else
            {
                SERVER_DEBUG("accept one client successfully (addr len is %d)\n", socklen);
                
                if (inet_ntop(AF_INET, &clientaddr.sin_addr, recvbuf, sizeof(recvbuf)))
                    SERVER_DEBUG("client addr is %s\n", recvbuf);
            }	
#endif
            do {
	    	    retlen = recv(app_sock, recvbuf, 1, MSG_DONTWAIT|MSG_PEEK);
	    	    SERVER_DEBUG("read len is %d\n", retlen);
	    	    
	    	    if (retlen < 0) {
	    	        SERVER_DEBUG("errno = %d\n", errno);
	    	    }
	    	    sleep(2);
            } while (1);
#if 0
            //write data to client
            sendlen = SEND_BUF_SIZE -1;;
            retlen =  0;
            ptr = sendbuf;
        
            do {
                retlen = write(app_sock, ptr, 1000);
                //sleep(1);

                printf("[server] retlen = %d for write!\n", retlen);
                sendlen -= retlen;
            } while( sendlen > 0);
            printf("[server] all data has been written!\n");
            sleep(20);
            
                        //write data to client
            sendlen = SEND_BUF_SIZE -1;;
            retlen =  0;
            ptr = sendbuf;
        
            do {
                retlen = write(app_sock, ptr, 1000);
                //sleep(1);

                printf("[server] retlen = %d for write!\n", retlen);
                sendlen -= retlen;
            } while( sendlen > 0);
            exit(1);
#else
 	while (1) {	
            //receive data
            recvlen = 0;
            retlen = 0;
            ptr = recvbuf;
            leftlen = RECV_BUF_SIZE -1;
            
	    do {
                printf("[Server] starting to receive data ...pid = %d\n", getpid());
	    	retlen = recv(app_sock, ptr, RECV_BUF_SIZE, MSG_DONTWAIT|MSG_PEEK);
//		retlen = read(app_sock,ptr, leftlen);
                printf("[Server] starting to receive data ...%d\n", retlen);
//		retlen = read(app_sock, ptr, RECV_BUF_SIZE);
     //   retlen = write(app_sock, ptr, RECV_BUF_SIZE);
//                retlen = recv(app_sock, ptr, leftlen, 0) ;
//		retlen = recvfrom(app_sock, ptr, leftlen, 0, (struct sockaddr *)& clientaddr, &socklen);
	
#if 0
//		printf("client_addr is %s\n", inet_ntoa(clientaddr.sin_addr));

//		printf("[server] retlen = %d\n", retlen);
                if(retlen < 0)
                {
                    printf("[Server] retlen < 0 \n");
                    if(errno == EINTR)
                        retlen = 0;
                    else
                        exit(1);
                }
                else if (retlen == 0)
                {
                    printf("[Server] connection has been closed by client\n");
		    exit(0);
                }
                recvlen += retlen;
                leftlen -= retlen;
             //   ptr += retlen;
#endif
            }while(leftlen > 0);
	}
	        printf("close the socket, since there is no other data \n");
#endif
            close(app_sock);
            printf("[Server] received total data len is : %d\n", recvlen);
        }
    }
    close(server_sock);
    return 0;   

#if 0
     sprintf(sendbuf, "welcome %s:%d here!/n", inet_ntoa(clientaddr.sin_addr.s_addr), clientaddr.sin_port);  
     //send data  
     sendlen = strlen(sendbuf) +1;  
     retlen = 0;  
     leftlen = sendlen;  
     ptr = sendbuf;  
     //while(leftlen)  
     {  
         retlen = send(app_sock, ptr, sendlen, 0);  
      if(retlen < 0)  
      {  
          if(errno == EINTR)  
            retlen = 0;  
        else  
            exit(1);  
      }  
      leftlen -= retlen;  
      ptr += retlen;  
     }  
#endif
}  
