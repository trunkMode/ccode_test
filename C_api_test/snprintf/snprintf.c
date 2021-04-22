#include <unistd.h>
#include <unistd.h>
#include <stdarg.h>
      make_message(const char *fmt, ...)
       {
           int size = 0;
           char *p = NULL;
           va_list ap;

           /* Determine required size */
printf("size = %d\n", size);
           va_start(ap, fmt);
           size = vsnprintf(p, size, fmt, ap);
           va_end(ap);

           if (size < 0)
               return NULL;
printf("size = %d\n", size);
           size++;             /* For '\0' */
printf("size = %d\n", size);
           p = malloc(size);
           if (p == NULL)
               return NULL;

           va_start(ap, fmt);
           size = vsnprintf(p, size, fmt, ap);
           va_end(ap);

           if (size < 0) {
               free(p);
               return NULL;
           }

           return p;
       }


int main()
{
	char buf[10];
	char *p = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

    memset(buf, 0x11, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s %s", p, p);
	printf("buf = %s\n", buf);
	printf("hello world\n");

    char *s = make_message("%s", p);
    printf("s = %s\n", s);
	return 0;
}
