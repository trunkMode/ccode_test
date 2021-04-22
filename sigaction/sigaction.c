#include <unistd.h>
#include <signal.h>
static exit_flag = 0;

static void installSignalHandler(int signo, const struct sigaction * act, int force)
{
    struct sigaction sa;
    sigaction(signo, NULL, &sa);
    
    if (force || sa.sa_handler == SIG_DFL || sa.sa_handler == SIG_IGN) {
	printf("%s %d: force = %d...\n", __func__, __LINE__, force);
	if (sa.sa_handler == SIG_DFL)
		printf("%s:%d: SIG_DFL\n", __func__, __LINE__);
	if (sa.sa_handler == SIG_IGN)
		printf("%s:%d: SIG_IGN\n", __func__, __LINE__);
        sigaction(signo, act, NULL);
    }
}

static void OS_SigpipeHandler(int signal)
{

	printf("%s : signal = %d\n", __func__, signal);
}
static void OS_Sigusr1Handler(int signal)
{
	printf("%s : signal = %d\n", __func__, signal);
	sleep(10);
	printf("%s %d\n", __func__, __LINE__);
}
static void OS_InstallSignalHandlers(int force)
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    /*mask SIGNINT during installed signal handler*/
    sigaddset(&sa.sa_mask, SIGINT);
    sa.sa_flags = 0;

//    sa.sa_handler = OS_SigpipeHandler;
//    installSignalHandler(SIGINT, &sa, force);

    sa.sa_handler = OS_Sigusr1Handler;
    installSignalHandler(SIGUSR1, &sa, force);
    
    //installSignalHandler(SIGTERM, &sa, 0);
}
static void my_sigusr2_handler(int signal)
{
	printf("%s : signal = %d\n", __func__, signal); 
}

static void my_sigterm_handler(int signal)
{
	printf("%s : signal = %d\n", __func__, signal);
	exit_flag = 1;
}
int main()
{
	signal(SIGTERM, my_sigterm_handler);
	signal(SIGUSR2, my_sigusr2_handler);
	OS_InstallSignalHandlers(0);
	while (1) 
	{
		if (exit_flag == 1)
			exit(0);
		sleep(1);
	}
}
