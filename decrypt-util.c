#define CIPHER GCRY_CIPHER_AES256

static void
decrypt (FILE            *in,
         FILE            *out,
         gcry_cipher_hd_t handle,
         size_t           blklen)
{
    char *iv = (char *) malloc (blklen * sizeof (char));
    fread (iv, blklen, 1, in);
    gcry_cipher_reset (handle);
    gcry_cipher_setiv (handle, iv, blklen);

    char len_buffer[8];
    fread (len_buffer, 8, 1, in);
    size_t real_len = atoi (len_buffer);

    fseek (in, 0, SEEK_END);
    size_t len = ftell (in);
    fseek (in, 0, SEEK_SET);

    char *raw_content = (char *) malloc ((len) * sizeof (char));
    fread (raw_content, len, 1, in);
    fclose (in);

    char *content = raw_content + blklen + 8;
    len -= (blklen + 8);

    gcry_cipher_decrypt (handle, content, len, NULL, 0);
    fwrite (content, real_len, 1, out);
    fclose (out);

    free (iv);
    free (raw_content);
}

