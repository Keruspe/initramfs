#include "run_bg.h"

#include <unistd.h>

void
mdadm_activate()
{
	if (access("/sbin/mdadm", X_OK) == 0)
		exec_bg_and_wait("/sbin/mdadm", "/sbin/mdadm", "--assemble", "--scan", NULL);
}

void
lvm_activate()
{
	if (access("/sbin/lvm", X_OK) == 0)
		exec_bg_and_wait("/sbin/lvm", "/sbin/lvm", "vgchange", "--sysinit", "--noudevsync", "-ay", NULL);
}
