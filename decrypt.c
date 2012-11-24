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

    for (size_t i = 0; i < len; ++i)
        fputc (content[i], stdout);

    free (content);
    gcry_cipher_close(handle);
    free (key);

    return 0;
}
