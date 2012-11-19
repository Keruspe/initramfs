#include <libcryptsetup.h>
#include <gpgme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>

#define DEFAULT_ROOT_PATH "/dev/mapper/root-luks"
#define DEFAULT_INIT_PATH "/sbin/init"
#define DEFAULT_FILESYSTEM_TYPE "ext4"

#define LUKS_HEADER_PLAIN "/root/luks_header"
#define LUKS_HEADER_ENCRYPTED LUKS_HEADER_PLAIN ".gpg"
#define LUKS_PASSFILE_PLAIN "/root/luks_passfile"
#define LUKS_PASSFILE_ENCRYPTED LUKS_PASSFILE_PLAIN ".gpg"
#define LUKS_DATA_DEVICE "/dev/sda5"

#define MAX_INIT_PATH_SIZE 25

static int 
is_blank (char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == EOF);
}

static char *
get_value (FILE *f) {
    char *path = (char *) malloc (10 * sizeof (char));
    int i = 0;
    char c;
    while (!is_blank (c = fgetc (f)))
    {
        path[i] = c;
        if (++i % 10 == 0)
            path = (char *) realloc (path, (i + 10) * sizeof (char));
    }
    if (i == 0)
    {
        free (path);
        return NULL;
    }
    path[i] = '\0';
    return path;
}

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

    struct crypt_device *cd = NULL;
    crypt_init(&cd, LUKS_HEADER_PLAIN);
    crypt_load(cd, CRYPT_LUKS1, NULL);
    crypt_set_data_device(cd, LUKS_DATA_DEVICE);
    crypt_set_password_retry(cd, 1);
    crypt_activate_by_keyfile_offset(cd, DEFAULT_ROOT_PATH, 0, LUKS_PASSFILE_PLAIN, 0, 0, 0);
    crypt_free (cd);

    char *root_path = NULL;
    FILE *cmdline = fopen ("/proc/cmdline", "r");
    if (cmdline)
    {
        char c;
        while ((c = fgetc (cmdline)) != EOF)
        {
            if (c == 'r' && fgetc (cmdline) == 'o' && fgetc (cmdline) == 'o' && fgetc (cmdline) == 't' && fgetc (cmdline) == '=')
                root_path = get_value (cmdline);
        }
        fclose (cmdline);
    }

    mount ((root_path) ? root_path : DEFAULT_ROOT_PATH, "/root", DEFAULT_FILESYSTEM_TYPE, MS_RDONLY, NULL);
    
    if (root_path)
        free (root_path);
    
    umount ("/proc");
    umount ("/sys");
    umount ("/dev");
    
    if (chdir ("/root") != 0) return -1;
    mount (".", "/", NULL, MS_MOVE, NULL);
    if (chroot (".") != 0) return -1;
    if (chdir ("/") != 0) return -1;
    
    execl (DEFAULT_INIT_PATH, DEFAULT_INIT_PATH, NULL);
}
