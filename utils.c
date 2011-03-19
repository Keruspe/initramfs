#include <unistd.h>
#include <sys/mount.h>

void
switch_root(char * init_path)
{
	if (chdir("/root") != 0) return;
	mount(".", "/", NULL, MS_MOVE, NULL);
	if (chroot(".") != 0) return;
	if (chdir("/") != 0) return;
	execl(init_path, init_path, NULL);
}
