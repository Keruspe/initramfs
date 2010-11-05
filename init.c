#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/wait.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

int
init()
{
	/**
	 * All the basic stuff before the sitch_root is made here
	 * A separate function is needed because for now, lvm needs an execl
	 */
	pid_t pid = 0;
	if (! (pid = fork()))
	{
		/*
		 * Here we need to fork too because we need 2 lvm calls
		 * First part of the stuff goes here
		 */
		fprintf(OUTPUT, "Initramfs booting...\n");
		/* Mount /proc and /sys */
		mount("none", "/proc", "proc", 0, NULL);
		mount("none", "/sys", "sysfs", 0, NULL);

		fprintf(OUTPUT, "Finding lvm devices...\n");
		/* Scan for volume groups */
		execl("/sbin/lvm", "/sbin/lvm", "vgscan", "--mknodes", NULL);

		return 0;
	}

	waitpid(pid, NULL, 0); /* Wait for the scan to finish */
	/* Activate the volume groups */
	execl("/sbin/lvm", "/sbin/lvm", "vgchange", "-ay", NULL);

	return 0;
}

char *
get_root_path()
{
	/* TODO: handle kernel command line */
	return DEFAULT_ROOT_PATH;
}

char *
get_init_path()
{
	/* TODO: handle kernel command line */
	return DEFAULT_INIT_PATH;
}

void
rm_rf (char * dir_name)
{
	/* Recursively remove directory */
	DIR * dir = opendir(dir_name);
	if (dir)
	{
		if (! chdir(dir_name))
			return;
		struct dirent * d;
		while ((d = readdir(dir)))
		{
			char * f = d->d_name;
			/* Don't delete . or .. */
			if (f[0] == '.' && (!f[1] || (f[1] == '.' && !f[2])))
				continue;
			rm_rf (f);
		}
		closedir(dir);
		if (chdir(".."))
			rmdir(dir_name);
	}
}

void
switch_root()
{
	if (! chdir("/root"))
		return;
	mount(".", "/", NULL, MS_MOVE, NULL);
	if (! chroot("."))
		return;
	if (! chdir("/"))
		return;
}

int
main()
{
	pid_t pid = 0;
	if (! (pid = fork()))
		return init(); /* Do the first part of the basic stuff in a fork */
	char * root_path = get_root_path();
	char * init_path = get_init_path();
	waitpid(pid, NULL, 0); /* Wait for the lvm volume groups to be activated */
	fprintf(OUTPUT, "Mounting root device: %s...\n", root_path);
	/* Mount / */
	mount(root_path, "/root", ROOT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
	fprintf(OUTPUT, "Unmounting /sys and /proc ...\n");
	/* Cleanup */
	umount("/sys");
	umount("/proc");
	rm_rf ("/dev");
	fprintf(OUTPUT, "### resuming normal boot ###\n");
	switch_root();
	execl(init_path, init_path, NULL);
}
