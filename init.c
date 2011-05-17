#include "cmdline.h"

#include <stdarg.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>

#define DEFAULT_ROOT_PATH "/dev/lvm/gentoo"
#define DEFAULT_INIT_PATH "/bin/systemd"
#define ROOT_FILESYSTEM_TYPE "ext4"

#define MAX_ARGS 31

void
exec_bg_and_wait(char * path, char * arg, ...)
{
    pid_t pid = fork();
    if (!pid) {
        char * argv[MAX_ARGS + 1];
        int argno = 0;
        va_list al;
        va_start(al, arg);
        while (arg && argno < MAX_ARGS)
        {
            argv[argno++] = arg;
            arg = va_arg(al, char *);
        }
        argv[argno] = NULL;
        va_end(al);
        execv(path, argv);
        exit(0);
    } else waitpid(pid, NULL, 0);
}

int
main()
{
    mount("none", "/proc", "proc", 0, NULL);
    mount("none", "/sys", "sysfs", 0, NULL);
    mount("none", "/dev", "devtmpfs", 0, NULL);
    if (access("/sbin/mdadm", X_OK) == 0)
        exec_bg_and_wait("/sbin/mdadm", "/sbin/mdadm", "--assemble", "--scan", NULL);
    if (access("/sbin/lvm", X_OK) == 0)
        exec_bg_and_wait("/sbin/lvm", "/sbin/lvm", "vgchange", "--sysinit", "--noudevsync", "-ay", NULL);
    Cmdline paths = parse_kernel_cmdline();
    char * root_path = paths.root ? paths.root : DEFAULT_ROOT_PATH;
    char * init_path = (paths.init[0] != '\0') ? paths.init : DEFAULT_INIT_PATH;
    mount(root_path, "/root", ROOT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
    if (paths.root)
        free (root_path);
    umount("/proc");
    umount("/sys");
    umount("/dev");
    if (chdir("/root") != 0) return -1;
    mount(".", "/", NULL, MS_MOVE, NULL);
    if (chroot(".") != 0) return -1;
    if (chdir("/") != 0) return -1;
    execl(init_path, init_path, NULL);
}
