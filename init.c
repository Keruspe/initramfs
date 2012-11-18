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

#define MAX_INIT_PATH_SIZE 25

typedef struct {
    char init[MAX_INIT_PATH_SIZE+1]; /* Or we'll have a leak */
    char *root;
    char *fs;
} Cmdline;

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

    Cmdline cmd = {
        .root = NULL,
        .fs = NULL,
        .init = ""
    };

    FILE *cmdline = fopen ("/proc/cmdline", "r");
    if (cmdline)
    {
        char c;
        while ((c = fgetc (cmdline)) != EOF)
        {
            if (c == 'i' && fgetc (cmdline) == 'n' && fgetc (cmdline) == 'i' && fgetc (cmdline) == 't' && fgetc (cmdline) == '=')
            {
                char *tmp = get_value (cmdline);
                if (!tmp)
                    continue;
                else if (strlen (tmp) < MAX_INIT_PATH_SIZE)
                    strcpy (cmd.init, tmp);
                free (tmp);
            }
            else if (c == 'r' && fgetc (cmdline) == 'o' && fgetc (cmdline) == 'o' && fgetc (cmdline) == 't' && fgetc (cmdline) == '=')
                cmd.root = get_value (cmdline);
            else if (c == 'f' && fgetc (cmdline) == 's' && fgetc(cmdline) == '=')
                cmd.fs = get_value (cmdline);
        }
        fclose (cmdline);
    }

    char *root_path = cmd.root ? cmd.root : DEFAULT_ROOT_PATH;
    char *root_fs = cmd.fs ? cmd.fs : DEFAULT_FILESYSTEM_TYPE;
    char *init_path = (cmd.init[0] != '\0') ? cmd.init : DEFAULT_INIT_PATH;
    
    mount (root_path, "/root", root_fs, MS_RDONLY, NULL);
    
    if (cmd.root)
        free (root_path);
    if (cmd.fs)
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
