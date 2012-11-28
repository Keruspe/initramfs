#include <libcryptsetup.h>
#include <gcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>

#define LUKS_HEADER_PLAIN "/root/luks_header"
#define LUKS_HEADER_ENCRYPTED LUKS_HEADER_PLAIN ".aes"
#define LUKS_PASSFILE_PLAIN "/root/luks_passfile"
#define LUKS_PASSFILE_ENCRYPTED LUKS_PASSFILE_PLAIN ".aes"
#define LUKS_DATA_DEVICE "/dev/sdb"
#define LUKS_VOLUME "root-luks"

#define ROOT_PATH "/dev/sda5"
#define INIT_PATH "/sbin/init"
#define FILESYSTEM_TYPE "ext4"

#include "decrypt-util.c"

#define AES_DECRYPT(in, out) \
    decrypt (fopen (in, "rb"), \
             fopen (out, "wb"), \
             handle, \
             blklen);

#define UMOUNT(dir) \
    umount (dir); \
    rmdir (dir);

static void
basic_mounts ()
{
    mount ("none", "/proc", "proc", 0, NULL);
    mount ("none", "/sys", "sysfs", 0, NULL);
    mount ("none", "/dev", "devtmpfs", 0, NULL);
}

int
main (void)
{
    basic_mounts ();

    #include "crypt-init.c"

    AES_DECRYPT (LUKS_HEADER_ENCRYPTED,
                 LUKS_HEADER_PLAIN)
    AES_DECRYPT (LUKS_PASSFILE_ENCRYPTED,
                 LUKS_PASSFILE_PLAIN)

    #include "crypt-deinit.c"

    struct crypt_device *cd = NULL;
    crypt_init(&cd, LUKS_HEADER_PLAIN);
    crypt_load(cd, CRYPT_LUKS1, NULL);
    crypt_set_data_device(cd, LUKS_DATA_DEVICE);
    crypt_set_password_retry(cd, 1);
    crypt_activate_by_keyfile_offset(cd, LUKS_VOLUME, 0, LUKS_PASSFILE_PLAIN, 0, 0, 0);
    crypt_free (cd);

    mount (ROOT_PATH, "/root", FILESYSTEM_TYPE, MS_RDONLY, NULL);
    
    UMOUNT ("/proc")
    UMOUNT ("/sys")
    UMOUNT ("/dev")
    
    if (chdir ("/root")) return -1;
    mount (".", "/", NULL, MS_MOVE, NULL);
    if (chroot (".")) return -1;
    if (chdir ("/")) return -1;

    basic_mounts ();
    
    mount ("/dev/mapper/root-luks", "/mnt/usbstick", FILESYSTEM_TYPE, MS_RDONLY, NULL);

    return execl (INIT_PATH, INIT_PATH, NULL);
}
