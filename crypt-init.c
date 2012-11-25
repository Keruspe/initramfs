    size_t blklen = gcry_cipher_get_algo_blklen (CIPHER);
    size_t keylen = gcry_cipher_get_algo_keylen (CIPHER);
    char *key = "coucou"; //getpass ("Passphrase: ");

    gcry_cipher_hd_t handle;
    gcry_cipher_open (&handle, CIPHER, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE|GCRY_CIPHER_CBC_CTS);
    gcry_cipher_setkey (handle, key, keylen);
