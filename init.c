#include "cmdline.h"

#include <unistd.h>
#include <sys/mount.h>

#define DEFAULT_ROOT_PATH "/dev/mapper/root-luks"
#define DEFAULT_INIT_PATH "/sbin/init"
#define DEFAULT_FILESYSTEM_TYPE "ext4"

int
main (void)
{
    mount ("none", "/proc", "proc", 0, NULL);
    mount ("none", "/sys", "sysfs", 0, NULL);
    mount ("none", "/dev", "devtmpfs", 0, NULL);

    Cmdline cmdline = parse_kernel_cmdline ();
    char *root_path = cmdline.root ? cmdline.root : DEFAULT_ROOT_PATH;
    char *root_fs = cmdline.fs ? cmdline.fs : DEFAULT_FILESYSTEM_TYPE;
    char *init_path = (cmdline.init[0] != '\0') ? cmdline.init : DEFAULT_INIT_PATH;
    
    mount (root_path, "/root", root_fs, MS_RDONLY, NULL);
    
    if (cmdline.root)
        free (root_path);
    if (cmdline.fs)
        free (root_fs);
    
    umount ("/proc");
    umount ("/sys");
    umount ("/dev");
    
    if (chdir ("/root") != 0) return -1;
    mount (".", "/", NULL, MS_MOVE, NULL);
    if (chroot (".") != 0) return -1;
    if (chdir ("/") != 0) return -1;
    
    execl (init_path, init_path, NULL);
}
