#include <gcrypt.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CIPHER GCRY_CIPHER_AES256

int
main (int argc, char *argv[])
{
    const char *output_file = NULL;
    FILE *in = NULL;

    switch (argc)
    {
    case 3:
        in = fopen (argv[1], "r");
    case 2:
        output_file = argv[argc - 1];
        break;
    default:
        fprintf (stderr, "Need either one or two arguments: <output file> or <input file> <output file>\n");
        return 1;
    }

    if (!in)
    {
        if (isatty (fileno (stdin)))
        {
            fprintf (stderr, "You need to pipe content if you only specify an output file or if input file is not readable\n");
            return 1;
        }
        in = stdin;
    }

    size_t len = 0;
    size_t total_size = 4096;
    char *content = (char *) malloc (total_size * sizeof (char));
    for (char c; (c = fgetc (in)) != EOF; ++len)
    {
        if (len && (len % 4096) == 0)
        {
            total_size += 4096;
            content = (char *) realloc (content, total_size * sizeof (char));
        }
        content[len] = c;
    }

    size_t keylen = gcry_cipher_get_algo_keylen (CIPHER);
    size_t blklen = gcry_cipher_get_algo_blklen (CIPHER);
    char *key = getpass ("Passphrase: ");

    char *iv = (char *) malloc (blklen * sizeof (char));
    srand (time (NULL));
    for (size_t i = 0; i < blklen; ++i)
    {
        iv[i] = (char) rand ();
        if (iv[i] == '\n')
            --i;
    }
 
    gcry_cipher_hd_t handle;
    gcry_cipher_open (&handle, CIPHER, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE|GCRY_CIPHER_CBC_CTS);
    gcry_cipher_setkey (handle, key, keylen);
    gcry_cipher_setiv (handle, iv, blklen);
    gcry_cipher_encrypt (handle, content, total_size, NULL, 0);

    unlink (output_file);
    FILE *out = fopen (output_file, "w+");
    fprintf (out, "%5lu%s\n", len, iv);
    for (size_t i = 0; i < total_size; ++i)
        fputc (content[i], out);
    fclose (out);

    gcry_cipher_close (handle);
    free (key);
    free (iv);
    free (content);

    return 0;
}
