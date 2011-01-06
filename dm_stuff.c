#include "run_bg.h"

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
	exec_bg_and_wait("/sbin/mdadm", "/sbin/mdadm", "--assemble", "--scan", NULL);
}

static void
lvm_scan()
{
	fprintf(OUTPUT, "Scanning for lvm devices...\n");
	exec_bg_and_wait("/sbin/lvm", "/sbin/lvm", "vgscan", "--mknodes", NULL);
}

void
lvm_activate()
{
	if (access("/sbin/lvm", X_OK) != 0)
		return;
	lvm_scan();
	fprintf(OUTPUT, "Activating lvm devices...\n");
	exec_bg_and_wait("/sbin/lvm", "/sbin/lvm", "vgchange", "-ay", NULL);
}
