#include <unistd.h>
#include <stdio.h>

#define FILE_NAME 	"test.txt"
char *read_file(const char *filename, size_t *len)
{
	size_t size;
	char *buf = NULL;
	FILE *fp = NULL;
	
	if (!filename)
		return NULL;

	fp = fopen(filename, "r");
	if (!fp) {
		printf("Failed to open file %s\n", filename);
		return NULL;
	}

	size = 0;
	printf("feof(fp) = %d\n", feof(fp));
	printf("ftell(fp) = %d\n", ftell(fp));
    fread(size, 1, 1, fp);
    printf("feof(fp) = %d\n", feof(fp));
	printf("ftell(fp) = %d\n", ftell(fp));    
    
	while (!feof(fp) && !ferror(fp)) {
		buf = realloc(buf, size + 4096);
		if (!buf) {
			free(buf);
			fclose(fp);
			return NULL;
		}
		size += fread(buf + size, 1, 4096, fp);
	}

	*len = size;
	fclose(fp);
	return buf;	
}

void fopen_write_file_test()
{
    int i = 0,j = 0;
    FILE *fp1 = fopen("test.txt", "w");

    fprintf(fp1, "pid is %d\n", getpid());
#if 1
    /* check if we can dump the file just  written */
	size_t len;
	char *buf;

	buf = read_file(FILE_NAME, &len);
	printf("buf is %s(%s)\n", buf ? buf : "null", buf ? "not null" : "null");
	printf("len = %d\n", len);
#endif
    for (j = 0;j < 1000; j++) {
        fprintf(fp1, "fp1 wirte %d.. \n", i);i++;
    }

    fprintf(fp1, "fp1 write %d.. \n", i);i++;

    sleep(10);
    FILE *fp2 = fopen("test.txt", "w");
    fprintf(fp2, "fp2 write %d.. \n", i);i++;

    fprintf(fp1, "fp1 write %d.. \n", i);i++;
    fprintf(fp2, "fp2 write %d.. \n", i);i++;
    fclose(fp2);
    //fclose(fp1);
    fclose(fp1);
}

int main()
{
#if 0
	size_t len;
	char *buf;

	buf = read_file(FILE_NAME, &len);
	printf("buf is %s(%s)\n", buf ? buf : "null", buf ? "not null" : "null");
	printf("len = %d\n", len);
#else
    fopen_write_file_test();
#endif
}
