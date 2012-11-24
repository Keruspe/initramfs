#include <gcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "decrypt-util.c"

int
main (int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf (stderr, "Need eactly one argument: file to decrypt\n");
        return 1;
    }

    size_t blklen = gcry_cipher_get_algo_blklen (CIPHER);
    size_t keylen = gcry_cipher_get_algo_keylen (CIPHER);
    char *key = getpass ("Passphrase: ");

    gcry_cipher_hd_t handle;
    gcry_cipher_open (&handle, CIPHER, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE|GCRY_CIPHER_CBC_CTS);
    gcry_cipher_setkey (handle, key, keylen);

    decrypt (fopen (argv[1], "rb"), stdout, handle, blklen);

    gcry_cipher_close(handle);
    free (key);

    return 0;
}
