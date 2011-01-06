#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 31

void
run_bg(void (*fun)(), int wait)
{
	pid_t pid = fork();
	if (!pid) {
		fun();
		exit(0);
	} else if (wait)
		waitpid(pid, NULL, 0);
}

void
run_bg_with_argv(void (*fun)(void *), void * argv, int wait)
{
	pid_t pid = fork();
	if (!pid) {
		fun(argv);
		exit(0);
	} else if (wait)
		waitpid(pid, NULL, 0);
}

void
exec_bg_and_wait(char * path, char * arg, ...)
{
	pid_t pid = fork();
	if (!pid) {
		char * argv[MAX_ARGS + 1];
		int argno = 0;
		va_list al;
		va_start(al, arg);
		while (arg && argno < MAX_ARGS)
		{
			argv[argno++] = arg;
			arg = va_arg(al, char *);
		}
		argv[argno] = NULL;
		va_end(al);
		execv(path, argv);
		exit(0);
	} else waitpid(pid, NULL, 0);
}

