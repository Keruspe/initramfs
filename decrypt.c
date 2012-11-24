#include <gcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CIPHER GCRY_CIPHER_AES256

int
main (int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf (stderr, "Need eactly one argument: file to decrypt\n");
        return 1;
    }

    size_t keylen = gcry_cipher_get_algo_keylen (CIPHER);
    size_t blklen = gcry_cipher_get_algo_blklen (CIPHER);
    char *key = getpass ("Passphrase: ");

    gcry_cipher_hd_t handle;
    gcry_cipher_open (&handle, CIPHER, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE|GCRY_CIPHER_CBC_CTS);
    gcry_cipher_setkey (handle, key, keylen);

    FILE *in = fopen (argv[1], "r");
    char *iv = (char *) malloc (blklen * sizeof (char));
    fread (iv, blklen, 1, in);

    fseek (in, 0, SEEK_END);
    size_t len = ftell (in);
    fseek (in, 0, SEEK_SET);
    char *raw_content = (char *) malloc ((len + 1) * sizeof (char));
    fread (raw_content, len, 1, in);
    fclose (in);

    char *content = raw_content + blklen;
    len -= blklen;

    gcry_cipher_reset (handle);
    gcry_cipher_setiv (handle, iv, blklen);
    gcry_cipher_decrypt (handle, content, len, NULL, 0);

    free (iv);

    fwrite (content, len, 1, stdout);

    free (raw_content);
    gcry_cipher_close(handle);
    free (key);

    return 0;
}
