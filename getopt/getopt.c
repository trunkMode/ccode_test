#include <unistd.h>

int main(int argc, char *argv[])
{
	int c;

	/*optind is the index of next option to be parsed in argv array */
	/*opterr*/
	printf("[start] optind = %d, opterr = %d, optopt = %d(%c), optarg = %s\n", optind, opterr, optopt, optopt, optarg ? optarg : "NULL");
	/* to prevent error message output, set opterr to 0*/
	opterr = 1;	
	while ((c = getopt(argc, argv, "i:")) != -1) {
		printf("[while loop] optind = %d, opterr = %d, optopt = %d(%c), optarg = %s\n", optind, opterr, optopt, optopt, optarg ? optarg : "NULL");
		switch (c) {
		case 'i':
			printf("option is %c, option value is %s\n", c, optarg);
			break;
		case '?':
//			printf("optarg = %s,
		default:
			break;
		}
	}
}
