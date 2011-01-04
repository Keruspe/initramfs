#include "cmdline.h"
#include "dm_stuff.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

extern bool have_lvm;

int
init()
{
	/**
	 * All the basic stuff before the sitch_root is made here
	 * A separate function is needed because for now, lvm needs an execl
	 */
	init_dm();
	pid_t pid = 0;
	if (! (pid = fork()))
	{
		/**
		 * Here we need to fork too because we need 2 lvm calls
		 * First part of the stuff goes here
		 */
		fprintf(OUTPUT, "Initramfs booting...\n");
		/* Mount /proc and /sys */
		mount("none", "/proc", "proc", 0, NULL);
		mount("none", "/sys", "sysfs", 0, NULL);
		mini_mdev();

		fprintf(OUTPUT, "Finding lvm devices...\n");
		if (have_lvm) /* Scan for volume groups */
			lvm_stuff();

		return 0;
	}

	waitpid(pid, NULL, 0); /* Wait for the scan to finish */
	if (have_lvm) /* Activate the volume groups */ /*FIXME*/
		execl("/sbin/lvm", "/sbin/lvm", "vgchange", "-ay", NULL);

	return 0;
}

int
main()
{
	pid_t pid = 0;
	if (! (pid = fork()))
		return init(); /* Do the first part of the basic stuff in a fork */
	waitpid(pid, NULL, 0); /* Wait for the lvm volume groups to be activated */
	fprintf(OUTPUT, "Parsing kernel cmdline for root and init paths\n");
	Cmdline paths = parse_kernel_cmdline();
	char * root_path = paths.root ? paths.root : DEFAULT_ROOT_PATH;
	char * init_path = (paths.init[0] != '\0') ? paths.init : DEFAULT_INIT_PATH;
	fprintf(OUTPUT, "Mounting root device: %s...\n", root_path);
	/* Mount / */
	mount(root_path, "/root", ROOT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
	if (paths.root)
		free (root_path);
	fprintf(OUTPUT, "Unmounting /sys and /proc ...\n");
	/* Cleanup */
	umount("/sys");
	umount("/proc");
	rm_rf ("/dev");
	fprintf(OUTPUT, "Resuming normal boot with init: %s...\n", init_path);
	switch_root();
	execl(init_path, init_path, NULL);
}
