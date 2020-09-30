#include <unistd.h>
#include <netdb.h>


int main()
{

    struct servent *srv_ent = NULL;

    srv_ent = getservbyport(htons(21), "tcp");

    if (srv_ent)
        printf("name = %s\n", srv_ent->s_name);
    return 0;
}
