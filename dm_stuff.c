#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

bool have_lvm, have_mdadm;

void
init_dm()
{
	have_lvm = (access("/sbin/lvm", X_OK) == 0);
	have_mdadm = (access("/sbin/mdadm", X_OK) == 0);
}

int
mdadm_stuff()
{
	pid_t pid = 0;
	if (! (pid = fork()))
		execl("/sbin/mdadm", "/sbin/mdadm", "--assemble", "--scan", NULL);
	waitpid(pid, NULL, 0);
	return 0;
}

void
lvm_stuff()
{
	if (have_mdadm)
		mdadm_stuff();
	execl("/sbin/lvm", "/sbin/lvm", "vgscan", "--mknodes", NULL);
}
