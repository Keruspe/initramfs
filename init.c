#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_INIT_PATH_SIZE 25

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

#define BLOCK_DIR "/sys/class/block"
#define MAX_PATH_SIZE 256
#define MAX_BLOCKNAME_SIZE 12

#define mknod(n, t, maj, min) mknod(n, (mode_t)(t|0644), (dev_t)(maj<<8 | min))

bool have_lvm, have_mdadm;

typedef struct {
	char init[MAX_INIT_PATH_SIZE+1]; /* Or we'll have a leak */
	char * root;
} Cmdline;

void
mini_mdev()
{
	mkdir("/dev", 0755);
	DIR * dir = opendir(BLOCK_DIR);
	if (!dir || (chdir(BLOCK_DIR) != 0))
		return;
	struct dirent * d;
	char * f;
	char dev_path[MAX_PATH_SIZE];
	char vals[MAX_BLOCKNAME_SIZE];
	char dev[MAX_BLOCKNAME_SIZE];
	FILE * file;
	unsigned column_index;
	while ((d=readdir(dir)))
	{
		f = d->d_name;
		if (!f[0] || !f[1] || (f[0] != 'h' && f[0] != 's') || (f[1] != 'd'))
			continue;
		sprintf(dev_path, "%s/dev", f);
		file = fopen(dev_path, "r");
		if (!file || !fgets(vals, MAX_BLOCKNAME_SIZE - 1, file))
			continue;
		fclose(file);
		for (column_index = 0 ; vals[column_index] != ':' ; ++column_index);
		vals[column_index] = '\0';
		sprintf(dev, "/dev/%s", f);
		mknod(dev, S_IFBLK, atoi(vals), atoi(vals + column_index + 1));
	}
	closedir(dir);
}

bool
is_blank(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == EOF);
}

char *
get_value(FILE * f) {
	char * path = (char *) malloc(11*sizeof(char)); /* /sbin/init size */
	int i = 0;
	char c;
	while (!is_blank(c = fgetc(f)))
	{
		path[i] = c;
		if (++i%11 == 0)
			path = (char *) realloc(path, (i+11) * sizeof(char));
	}
	if (i == 0)
	{
		free (path);
		return NULL;
	}
	path[i] = '\0';
	return path;
}

Cmdline
parse_kernel_cmdline()
{
	Cmdline paths;
	paths.root = NULL;
	paths.init[0] = '\0';
	FILE * f = fopen("/proc/cmdline", "r");
	if (!f)
		return paths;
	char c;
	while ((c = fgetc(f)) != EOF)
	{
		if (c == 'i' && fgetc(f) == 'n' && fgetc(f) == 'i' && fgetc(f) == 't' && fgetc(f) == '=')
		{
			char * tmp = get_value(f);
			if (!tmp)
				continue;
			else if (strlen(tmp) > MAX_INIT_PATH_SIZE)
				free(tmp);
			else
				strcpy(paths.init, tmp);
		}
		else if (c == 'r' && fgetc(f) == 'o' && fgetc(f) == 'o' && fgetc(f) == 't' && fgetc(f) == '=')
			paths.root = get_value(f);
	}
	fclose(f);
	return paths;
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

int
init()
{
	/**
	 * All the basic stuff before the sitch_root is made here
	 * A separate function is needed because for now, lvm needs an execl
	 */
	have_lvm = (access("/sbin/lvm", X_OK) == 0);
	have_mdadm = (access("/sbin/mdadm", X_OK) == 0);
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
	if (have_lvm) /* Activate the volume groups */
		execl("/sbin/lvm", "/sbin/lvm", "vgchange", "-ay", NULL);

	return 0;
}

void
rm_rf (char * path)
{
	/* Recursively remove directory */
	DIR * dir = opendir(path);
	if (dir)
	{
		if (! chdir(path)) {}
		struct dirent * d;
		while ((d = readdir(dir)))
		{
			char * f = d->d_name;
			/* Don't delete . nor .. */
			if (f[0] == '.' && (!f[1] || (f[1] == '.' && !f[2])))
				continue;
			rm_rf (f);
		}
		closedir(dir);
		if (chdir("..")) {}
		rmdir(path);
	}
	else
		unlink(path);
}

void
switch_root()
{
	if (chdir("/root")) {}
	mount(".", "/", NULL, MS_MOVE, NULL);
	if (chroot(".")) {}
	if (chdir("/")) {}
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
