#include <gcrypt.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CIPHER GCRY_CIPHER_AES256

int
main (int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf (stderr, "Need exactly two arguments: <input file> <output file>\n");
        return 1;
    }

    FILE *in = fopen (argv[1], "rb");

    fseek (in, 0, SEEK_END);
    size_t len = ftell (in);
    fseek (in, 0, SEEK_SET);
    char *content = (char *) malloc ((len + 1) * sizeof (char));
    fread (content, len, 1, in);
    fclose (in);

    #include "crypt-init.c"

    char *iv = (char *) malloc (blklen * sizeof (char));
    srand (time (NULL));
    for (size_t i = 0; i < blklen; ++i)
    {
        iv[i] = (char) rand ();
        if (iv[i] == '\n')
            --i;
    }
 
    gcry_cipher_setiv (handle, iv, blklen);
    gcry_cipher_encrypt (handle, content, len, NULL, 0);

    #include "crypt-deinit.c"

    unlink (argv[2]);
    FILE *out = fopen (argv[2], "wb");
    fwrite (iv, blklen, 1, out);
    fwrite (content, len, 1, out);
    fclose (out);

    free (iv);
    free (content);

    return 0;
}
