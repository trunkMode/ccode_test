#include <stdio.h>

#include <stdlib.h>
main()
{
    int fd;
    char template[] = "template-XXXXXX";
    fd = mkstemp(template);
    printf("template = %s\n", template);
    close(fd);
}

