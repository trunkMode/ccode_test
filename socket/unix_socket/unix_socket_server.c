#include <unistd.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>


//#define UNIX_DOMAIN_PATH    "./unix_server"
#define UNIX_DOMAIN_PATH    "/home/alan/codeTest/ccode_test/socket/unix_socket/unix_server"
#define SERVER_DEBUG(FMAT,ARGS...) printf("[server]%s %d:"FMAT"\n", __func__, __LINE__, ## ARGS)

int main()
{
    int skfd;
    char buf[1024*1024];
    socklen_t sock_len;
    struct sockaddr_un  server;
    struct sockaddr_un  from;
    socklen_t size = 0;
 
    if ((skfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        SERVER_DEBUG("Failed to create socket\n");
        return -1;
    }

    memset(&server, 0x00, sizeof(server));
    server.sun_family = AF_UNIX;
    snprintf(server.sun_path, sizeof(server.sun_path), "%s", UNIX_DOMAIN_PATH);
#if 1
    size = offsetof(struct sockaddr_un, sun_path) + strlen(server.sun_path);
#else
    size = sizeof(server);
#endif

    unlink(UNIX_DOMAIN_PATH);
    if (bind(skfd, (struct sockaddr *)&server, size) < 0) {
        SERVER_DEBUG("Failed to bind socket to unix path!\n");
        perror("error:");
        return -1;
    }

    printf("**** server ****\n");
    while (1) {
        sock_len = sizeof(from);
        if (recvfrom(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &sock_len) < 0) {
            SERVER_DEBUG("Failed to recvfrom error!\n");
            return -1;
        }
        SERVER_DEBUG("Recv one connection !\n");
        if (sendto(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, sizeof(from)) < 0) {
            SERVER_DEBUG("Sent back connect!\n");
        }
    }
}
