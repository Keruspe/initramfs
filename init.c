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
#define LUKS_DATA_DEVICE "/dev/sda5"
#define LUKS_VOLUME "root-luks"

#define DEFAULT_ROOT_PATH "/dev/mapper/" LUKS_VOLUME
#define DEFAULT_INIT_PATH "/sbin/init"
#define DEFAULT_FILESYSTEM_TYPE "ext4"

#define CIPHER GCRY_CIPHER_AES256

static void
aes_decrypt (gcry_cipher_hd_t handle,
             size_t           blklen,
             const char      *encrypted_file,
             const char      *plain_file)
{
    FILE *in = fopen (encrypted_file, "r");
    char *iv = (char *) malloc (blklen * sizeof (char));
    size_t len;
    fscanf (in, "%5lu%s\n", &len, iv);

    size_t total_size = 4096 * ((len/4096) + 1);
    char *content = (char *) malloc (total_size * sizeof (char));
    for (size_t i = 0; i <= total_size; ++i)
        content[i] = fgetc (in);

    fclose (in);

    gcry_cipher_reset (handle);
    gcry_cipher_setiv (handle, iv, blklen);
    gcry_cipher_decrypt (handle, content, 512, NULL, 0);

    free (iv);

    FILE *out = fopen (plain_file, "w+");
    for (size_t i = 0; i < len; ++i)
        fputc (content[i], out);
    fclose (out);
    free (content);
}

static char *
get_value (FILE *f) {
    char *path = (char *) malloc (10 * sizeof (char));
    int i = 0;
    char c;
    while (!((c = fgetc (f)) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == EOF))
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

int
main (void)
{
    mount ("none", "/proc", "proc", 0, NULL);
    mount ("none", "/sys", "sysfs", 0, NULL);
    mount ("none", "/dev", "devtmpfs", 0, NULL);

    size_t keylen = gcry_cipher_get_algo_keylen (CIPHER);
    size_t blklen = gcry_cipher_get_algo_blklen (CIPHER);
    char *key = getpass ("Passphrase: ");

    gcry_cipher_hd_t handle;
    gcry_cipher_open (&handle, CIPHER, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE|GCRY_CIPHER_CBC_CTS);
    gcry_cipher_setkey (handle, key, keylen);

    aes_decrypt (handle,
                 blklen,
                 LUKS_HEADER_ENCRYPTED,
                 LUKS_HEADER_PLAIN);
    aes_decrypt (handle,
                 blklen,
                 LUKS_PASSFILE_ENCRYPTED,
                 LUKS_PASSFILE_PLAIN);

    gcry_cipher_close(handle);
    free (key);

    struct crypt_device *cd = NULL;
    crypt_init(&cd, LUKS_HEADER_PLAIN);
    crypt_load(cd, CRYPT_LUKS1, NULL);
    crypt_set_data_device(cd, LUKS_DATA_DEVICE);
    crypt_set_password_retry(cd, 1);
    crypt_activate_by_keyfile_offset(cd, LUKS_VOLUME, 0, LUKS_PASSFILE_PLAIN, 0, 0, 0);
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
