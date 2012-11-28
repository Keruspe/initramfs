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

    #include "crypt-init.c"

    FILE *in = fopen (argv[1], "rb");

    fseek (in, 0, SEEK_END);
    size_t real_len = ftell (in);
    size_t len = ((real_len / blklen) + 1) * blklen;
    fseek (in, 0, SEEK_SET);
    char *content = (char *) malloc ((len + 1) * sizeof (char));
    fread (content, len, 1, in);
    fclose (in);

    char *iv = (char *) malloc (blklen * sizeof (char));
    srand (time (NULL));
    for (size_t i = 0; i < blklen; ++i)
    {
        iv[i] = (char) rand ();
        if (iv[i] == '\n')
            --i;
    }
 
    gcry_cipher_setiv (handle, iv, blklen);

    fprintf (stderr, "len: %lu (%lu)\n", len, real_len);

    fprintf (stderr, "key: ");
    for (size_t i = 0; i < strlen (key); ++i)
        fprintf (stderr, "%d ", key[i]);
    fprintf (stderr, "\n");

    fprintf (stderr, "iv: ");
    for (size_t i = 0; i < blklen; ++i)
        fprintf (stderr, "%d ", iv[i]);
    fprintf (stderr, "\n");

    fprintf (stderr, "plain: ");
    for (size_t i = 0; i < len; ++i)
        fprintf (stderr, "%x ", content[i]);
    fprintf (stderr, "\n");

    gcry_cipher_encrypt (handle, content, len, NULL, 0);

    fprintf (stderr, "crypt: ");
    for (size_t i = 0; i < len; ++i)
        fprintf (stderr, "%x ", content[i]);
    fprintf (stderr, "\n");

#if 0
    gcry_cipher_reset (handle);
    gcry_cipher_setiv (handle, iv, blklen);
    gcry_cipher_decrypt (handle, content, len, NULL, 0);

    fprintf (stderr, "plain: ");
    for (size_t i = 0; i < len; ++i)
        fprintf (stderr, "%x ", content[i]);
    fprintf (stderr, "\n");
#endif

    #include "crypt-deinit.c"

    unlink (argv[2]);
    FILE *out = fopen (argv[2], "wb");
    fwrite (iv, blklen, 1, out);

    char len_buffer[9];
    sprintf (len_buffer, "%8lu", real_len);
    fwrite (len_buffer, 8, 1, out);
    fwrite (content, len, 1, out);
    fclose (out);

    free (iv);
    free (content);

    return 0;
}
