#include <unistd.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <pthread.h>


#define UNIX_DOMAIN_PATH    "/home/alan/codeTest/ccode_test/socket/unix_socket/unix_server"
#define DEBUG(FMAT,ARGS...) printf("[client]%s %d:"FMAT"\n", __func__, __LINE__, ## ARGS)

pthread_t tid_unix_sk[1];
void thread(void *arg);
int count = 0;
int main()
{
    int i;

    for (i = 0; i < sizeof(tid_unix_sk)/sizeof(pthread_t); i++) {
        pthread_create(&tid_unix_sk[i], NULL, thread, i);
    }

    for (i = 0; i < sizeof(tid_unix_sk)/sizeof(pthread_t); i++) {
        pthread_join(tid_unix_sk[i], NULL);
    }
}

void thread(void *arg)
{
    int i = (int)arg;
    int skfd = -1, cnt;
    /*note: if the buf is set to 1024*1024, the sendto() will fail due to too long message error.
     * the buf is longer than socket buf? */
    char buf[4*1024];
    socklen_t srv_sock_len, local_sock_len;
    struct sockaddr_un  local;
    struct sockaddr_un  server;
    struct sockaddr_un  from;

    /* create socket */
    if ((skfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket\n");
        return -1;
    }

    /* bind a local addr */
    memset(&local, 0x00, sizeof(local));
    local.sun_family = AF_UNIX;
    snprintf(local.sun_path, sizeof(local.sun_path), "%s-%d", UNIX_DOMAIN_PATH, i);
    local_sock_len = offsetof(struct sockaddr_un, sun_path) + strlen(local.sun_path);
    unlink(local.sun_path);

    if (bind(skfd, (struct sockaddr *)&local, local_sock_len) < 0) {
        DEBUG("Failed to bind socket to unix path (%s)!\n", local.sun_path);
        perror("error:");
        goto error;
    }

    /* sever addr */
    memset(&server, 0x00, sizeof(server));
    server.sun_family = AF_UNIX;
    snprintf(server.sun_path, sizeof(server.sun_path), "%s", UNIX_DOMAIN_PATH);
#if 0
    srv_sock_len = offsetof(struct sockaddr_un, sun_path) + strlen(server.sun_path);
#else
    srv_sock_len = sizeof(server);
#endif

    while (1) {
        //sock_len = sizeof(from);
        if (i < 3) {
            /* thread(0 ~ 2) send msg */
            cnt = sendto(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&server, srv_sock_len);
            if (cnt < 0) {
                DEBUG("Thread[%d] Failed to Sent msg server!\n", i);
                perror("error:");
            } else {
                DEBUG("Thread[%d] Send %d bytes message to server\n", i, cnt);
            }
        } else {
            /* thread(3) recv msg */
            if (recvfrom(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &local_sock_len) < 0) {
                DEBUG("Failed to recvfrom error!\n");
                return -1;
            } else {  
		printf("receive packets from %s\n", from.sun_path);
	    }
	 
        }
#if 0
        if (recvfrom(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &sock_len) < 0) {
            DEBUG("Failed to recvfrom error!\n");
            return -1;
        }
        DEBUG("Recv one connection !\n");
        if (sendto(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, sizeof(from)) < 0) {
            DEBUG("Sent back connect!\n");
        }
#endif
        sleep(2);
    }

error:
    if (skfd > 0)
        close(skfd);
    return -1;
}

