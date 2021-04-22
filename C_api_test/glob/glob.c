#include <unistd.h>
#include <glob.h>

#if 0
       #include <glob.h>

       int glob(const char *pattern, int flags,
		                       int (*errfunc) (const char *epath, int eerrno),
				                       glob_t *pglob);
       void globfree(glob_t *pglob);

#endif

int main()
{
	int i = 0;
	glob_t glob_buf;

	if (glob("/sys/class/*", GLOB_MARK, NULL, &glob_buf) != 0) {
		printf("glob failed \n");
		return -1;
	} 
		
	for (i = 0; i < glob_buf.gl_pathc; i++) {
		printf("gl_pathv[i] = %s\n", glob_buf.gl_pathv[i]);
	}
	return 0;
}
