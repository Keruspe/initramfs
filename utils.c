#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>

#define mknod(n, t, maj, min) mknod(n, (mode_t)(t|0644), (dev_t)(maj<<8 | min))

#define BLOCK_DIR "/sys/class/block"

#define MAX_PATH_SIZE 256
#define MAX_BLOCKNAME_SIZE 12

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
	unsigned int column_index;
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
	if (chdir("/") != 0) { /* return; */ }
	closedir(dir);
}

void
rm_rf (char * path)
{
	/* Recursively remove directory */
	DIR * dir = opendir(path);
	if (dir)
	{
		if (chdir(path) != 0) goto fail;
		struct dirent * d;
		while ((d = readdir(dir)))
		{
			char * f = d->d_name;
			/* Don't delete . nor .. */
			if (f[0] == '.' && (!f[1] || (f[1] == '.' && !f[2])))
				continue;
			rm_rf (f);
		}
		if (chdir("..") != 0) { /* goto fail; */ }
	fail:
		closedir(dir);
		rmdir(path);
	}
	else
		unlink(path);
}

void
switch_root(char * init_path)
{
	if (chdir("/root") != 0) return;
	mount(".", "/", NULL, MS_MOVE, NULL);
	if (chroot(".") != 0) return;
	if (chdir("/") != 0) return;
	execl(init_path, init_path, NULL);
}
