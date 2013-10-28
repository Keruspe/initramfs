#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>

int
main (int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    mknod ("/dev/sda5", S_IFBLK|0644, 8 << 8 | 5);
    mount ("/dev/sda5", "/root", "ext4", MS_RDONLY, NULL);
    unlink ("/dev/sda5");
    if (chdir("/root") != 0) return -1;
    mount(".", "/", NULL, MS_MOVE, NULL);
    if (chroot(".") != 0) return -1;
    if (chdir("/") != 0) return -1;
    execl("/sbin/init", "/sbin/init", NULL);
}
