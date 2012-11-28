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

    #include "crypt-init.c"

    decrypt (fopen (argv[1], "rb"), stdout, handle, blklen);

    #include "crypt-deinit.c"

    return 0;
}
