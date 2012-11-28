#define CIPHER GCRY_CIPHER_AES256
#define READ(buf, len, in) if (fread (buf, len, 1, in) != 1) goto out;

static void
decrypt (FILE            *in,
         FILE            *out,
         char            *key,
         gcry_cipher_hd_t handle,
         size_t           blklen)
{
    char *iv = (char *) malloc (blklen * sizeof (char));
    char *raw_content = NULL;
    READ (iv, blklen, in)

    gcry_cipher_reset (handle);
    gcry_cipher_setiv (handle, iv, blklen);

    char len_buffer[9];
    READ (len_buffer, 8, in)
    size_t real_len = atoi (len_buffer);

    fseek (in, 0, SEEK_END);
    size_t len = ftell (in);
    fseek (in, 0, SEEK_SET);

    raw_content = (char *) malloc ((len + 1) * sizeof (char));
    READ (raw_content, len, in)

    char *content = raw_content + blklen + 8;
    len -= (blklen + 8);

    fprintf (stderr, "len: %lu (%lu)\n", len, real_len);

    fprintf (stderr, "key: ");
    for (size_t i = 0; i < strlen (key); ++i)
        fprintf (stderr, "%d ", key[i]);
    fprintf (stderr, "\n");

    fprintf (stderr, "iv: ");
    for (size_t i = 0; i < blklen; ++i)
        fprintf (stderr, "%d ", iv[i]);
    fprintf (stderr, "\n");

    fprintf (stderr, "crypt: ");
    for (size_t i = 0; i < len; ++i)
        fprintf (stderr, "%x ", content[i]);
    fprintf (stderr, "\n");

    gcry_cipher_decrypt (handle, content, len, NULL, 0);

    fprintf (stderr, "plain: ");
    for (size_t i = 0; i < len; ++i)
        fprintf (stderr, "%x ", content[i]);
    fprintf (stderr, "\n");

    fwrite (content, real_len, 1, out);
    fclose (out);

out:
    if (raw_content)
        free (raw_content);
    free (iv);
    fclose (in);
}

