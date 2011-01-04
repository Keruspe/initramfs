#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define OUTPUT stdout

void
mdadm_activate()
{
	if (access("/sbin/mdadm", X_OK) != 0)
		return;
	fprintf(OUTPUT, "Activating mdadm RAID arrays...\n");
	pid_t pid = 0;
	if (! (pid = fork()))
		execl("/sbin/mdadm", "/sbin/mdadm", "--assemble", "--scan", NULL);
	else
		waitpid(pid, NULL, 0);
}

static void
lvm_scan()
{
	fprintf(OUTPUT, "Scanning for lvm devices...\n");
	pid_t pid = 0;
	if (! (pid = fork()))
		execl("/sbin/lvm", "/sbin/lvm", "vgscan", "--mknodes", NULL);
	else
		waitpid(pid, NULL, 0);
}

void
lvm_activate()
{
	if (access("/sbin/lvm", X_OK) != 0)
		return;
	lvm_scan();
	fprintf(OUTPUT, "Activating lvm devices...\n");
	pid_t pid = 0;
	if (! (pid = fork()))
		execl("/sbin/lvm", "/sbin/lvm", "vgchange", "-ay", NULL);
	else
		waitpid(pid, NULL, 0);
}
