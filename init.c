#include "cmdline.h"
#include "dm_stuff.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

int
main()
{
	fprintf(OUTPUT, "Initramfs booting...\n");
	mount("none", "/proc", "proc", 0, NULL);
	mount("none", "/sys", "sysfs", 0, NULL);
	mini_mdev();
	mdadm_activate();
	lvm_activate();
	Cmdline paths = parse_kernel_cmdline();
	char * root_path = paths.root ? paths.root : DEFAULT_ROOT_PATH;
	char * init_path = (paths.init[0] != '\0') ? paths.init : DEFAULT_INIT_PATH;
	fprintf(OUTPUT, "Mounting root device: %s...\n", root_path);
	mount(root_path, "/root", ROOT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
	if (paths.root)
		free (root_path);
	umount("/sys");
	umount("/proc");
	rm_rf ("/dev");
	fprintf(OUTPUT, "Resuming normal boot with init: %s...\n", init_path);
	switch_root();
	execl(init_path, init_path, NULL);
}
