#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <lvm2app.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

int
init()
{
	fprintf(OUTPUT, "Initramfs booting...\n");
	/* Mount /proc and /sys */
	mount("none", "/proc", "proc", 0, NULL);
	mount("none", "/sys", "sysfs", 0, NULL);

	fprintf(OUTPUT, "Finding lvm devices...\n");
	lvm_t lvmh = lvm_init(NULL);
	lvm_scan(lvmh);
	struct dm_list * vgs = lvm_list_vg_names(lvmh);
	vg_t vg;
	struct lvm_str_list * strl;
	struct dm_list * lvs;
	struct lvm_lv_list * lv_list;
	dm_list_iterate_items(strl, vgs)
	{
		vg = lvm_vg_open(lvmh, strl->str, "r", 0);
		lvs = lvm_vg_list_lvs(vg);
		dm_list_iterate_items(lv_list, lvs)
			lvm_lv_activate(lv_list->lv);
		lvm_vg_close(vg);
	}
	lvm_quit(lvmh);

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
	//rm_rf ("/dev");
	fprintf(OUTPUT, "### resuming normal boot ###\n");
	switch_root();
	execl(init_path, init_path, NULL);
}
