#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/wait.h>

#define MAX_INIT_PATH_SIZE 25

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/sbin/init"
#define ROOT_FILESYSTEM_TYPE "ext4"
#define OUTPUT stdout

typedef struct {
	char init[MAX_INIT_PATH_SIZE+1]; /* Or we'll have a leak */
	char * root;
} cmdline;

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

cmdline
parse_kernel_cmdline()
{
	cmdline paths;
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
			/* Don't delete . or .. */
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
	cmdline paths = parse_kernel_cmdline();
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
