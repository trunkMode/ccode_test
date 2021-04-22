#include <unistd.h>
#include <string.h>
#include <stdio.h>

/*
 * char *strtok(char *str, const char *delim);
 * char *strtok_r(char *str, const char *delim, char **saveptr);
 */

int main()
{
	char *save_ptr = NULL;
#if 1
	char *tmp_str = NULL;
	char *str = malloc(100);
	snprintf(str, 100, "%s", "array[10].test");
#else
	char str[] = "hello..world..test";
//	char *str = "hello..world.|.test           ";
#endif

	tmp_str = str;
	for (;; tmp_str = NULL) {
		char *k = strtok_r(tmp_str, "[].", &save_ptr);

		printf("k = %s\n", k?k:"null");
		
		if (!k)
			break;
	}

	free(str);
	return 0;
}

