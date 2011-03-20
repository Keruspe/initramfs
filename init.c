#include "cmdline.h"
#include "dm_stuff.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"

void
switch_root(char * init_path)
{
	if (chdir("/root") != 0) return;
	mount(".", "/", NULL, MS_MOVE, NULL);
	if (chroot(".") != 0) return;
	if (chdir("/") != 0) return;
	execl(init_path, init_path, NULL);
}

int
main()
{
	mount("none", "/proc", "proc", 0, NULL);
	mount("none", "/sys", "sysfs", 0, NULL);
	mount("none", "/dev", "devtmpfs", 0, NULL);
	mdadm_activate();
	lvm_activate();
	Cmdline paths = parse_kernel_cmdline();
	char * root_path = paths.root ? paths.root : DEFAULT_ROOT_PATH;
	char * init_path = (paths.init[0] != '\0') ? paths.init : DEFAULT_INIT_PATH;
	mount(root_path, "/root", ROOT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
	if (paths.root)
		free (root_path);
	umount("/proc");
	umount("/sys");
	umount("/dev");
	switch_root(init_path);
}
