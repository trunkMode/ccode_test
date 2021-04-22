#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
//#include <stropts.h>
#include <sys/poll.h>
//#include <sys/stropts.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <poll.h>
#include <netpacket/packet.h>   /* use PF_PACKET for recv/send SDP packets */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#define POLLERR		0x008		/* Error condition.  */
#define POLLHUP		0x010		/* Hung up.  */
#define POLLNVAL	0x020		/* Invalid polling request.  */

# define POLLMSG	0x400
# define POLLREMOVE	0x1000
# define POLLRDHUP	0x2000


#define BUFSIZE 1024

static int bind_instance_interface(int skfd)
{
    struct sockaddr_ll sll;
    int err;
    socklen_t errlen = sizeof(err);
    struct ifreq ifr;
   

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", "eth0");

    if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0) {
        printf("%s: ioctl(SIOCGIFINDEX)", __func__);
        return -1;
    }



    /* Bind to the specified device so we only see packets from it. */
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(skfd, (struct sockaddr *) &sll, sizeof(sll)) == -1)
    {
        return -1;
    }
#if 0
    /* Any pending errors, e.g., network is down? */
    if (getsockopt(skfd, SOL_SOCKET, SO_ERROR, &err, &errlen) || err)
    {
        return -1;
    }
#endif

    return 0;
}
int main(int argc, char *argv[])
{
    char buf[BUFSIZE];
    int bytes;
    struct pollfd *pollfd;
    int i=0;
    int nummonitor=0;
    int numready;
    int errno;
    char *str;

    int skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

    if (skfd < 0) {
        printf("Failed to create raw socket!\n");
        return -1;
    }

    bind_instance_interface(skfd);

    pollfd = (struct pollfd*) malloc(sizeof(struct pollfd));
    if(pollfd == NULL) {
        printf("Failed to malloc memory!\n");
        return -1;
    }
    
    for(i; i<1; i++) {
        (pollfd+i)->fd = skfd;

        if((pollfd+i)->fd >= 0)
            (pollfd+i)->events = POLLIN;
    }

    while (1) {
        //printf("#####%d##########\n", __LINE__);
        int ret;
        ret = poll(pollfd, 1, 1);
        /* If we were interrupted by a signal, start the loop over.  The user should call daq_breakloop to actually exit. */
        if (ret < 0 && errno != EINTR) {
            printf("Poll failed!\n");
            return -1;
        }
        /* If the poll times out, return control to the caller. */
        if (ret == 0)
            continue;
        /* If some number of of sockets have events returned, check them all for badness. */
        if (ret > 0)
        {
            for (i = 0; i < 1; i++)
            {
                if (pollfd[i].revents & (POLLHUP | POLLRDHUP | POLLERR | POLLNVAL))
                {
                    if (pollfd[i].revents & (POLLHUP | POLLRDHUP))
                        printf("Hang-up on a packet socket\n");
                    else if (pollfd[i].revents & POLLERR)
                        printf("Encountered error condition on a packet socket\n");
                    else if (pollfd[i].revents & POLLNVAL)
                        printf("Invalid polling request on a packet socket\n");
                 }
            }
        }
    }
    free(pollfd);
    return 0;
}

