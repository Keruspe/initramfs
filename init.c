#include "cmdline.h"

#include <gpgme.h>
#include <unistd.h>
#include <sys/mount.h>

#define DEFAULT_ROOT_PATH "/dev/mapper/root-luks"
#define DEFAULT_INIT_PATH "/sbin/init"
#define DEFAULT_FILESYSTEM_TYPE "ext4"

#define LUKS_HEADER_PLAIN "/root/luks_header"
#define LUKS_HEADER_ENCRYPTED LUKS_HEADER_PLAIN ".gpg"
#define LUKS_PASSFILE_PLAIN "/root/luks_passfile"
#define LUKS_PASSFILE_ENCRYPTED LUKS_PASSFILE_PLAIN ".gpg"

static void
gpgme_decrypt (gpgme_ctx_t context,
               const char *encrypted_file,
               const char *plain_file)
{
    gpgme_data_t encrypted, plain;
    FILE *ef = fopen (encrypted_file, "r");
    FILE *pf = fopen (plain_file, "w+");
    gpgme_data_new_from_stream (&encrypted, ef);
    gpgme_data_new_from_stream (&plain, pf);

    gpgme_op_decrypt (context, encrypted, plain);

    gpgme_data_release (plain);
    gpgme_data_release (encrypted);
    fclose (ef);
    fclose (pf);
}

int
main (void)
{
    mount ("none", "/proc", "proc", 0, NULL);
    mount ("none", "/sys", "sysfs", 0, NULL);
    mount ("none", "/dev", "devtmpfs", 0, NULL);

    gpgme_ctx_t context;
    gpgme_check_version (NULL);
    gpgme_new (&context);

    gpgme_decrypt (context,
                   LUKS_HEADER_ENCRYPTED,
                   LUKS_HEADER_PLAIN);
    gpgme_decrypt (context,
                   LUKS_PASSFILE_ENCRYPTED,
                   LUKS_PASSFILE_PLAIN);

    gpgme_release (context);

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
