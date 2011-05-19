#include "cmdline.h"

#include <stdio.h>
#include <string.h>

static int 
is_blank(char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == EOF);
}

static char *
get_value(FILE * f) {
    char * path = (char *) malloc(13*sizeof(char)); /* /bin/systemd size */
    int i = 0;
    char c;
    while (!is_blank(c = fgetc(f)))
    {
        path[i] = c;
        if (++i%13 == 0)
            path = (char *) realloc(path, (i+13) * sizeof(char));
    }
    if (i == 0)
    {
        free (path);
        return NULL;
    }
    path[i] = '\0';
    return path;
}

Cmdline
parse_kernel_cmdline()
{
    Cmdline cmdline = {
        .root = NULL,
        .fs = NULL,
        .init = ""
    };
    FILE * f = fopen("/proc/cmdline", "r");
    if (!f)
        return cmdline;
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == 'i' && fgetc(f) == 'n' && fgetc(f) == 'i' && fgetc(f) == 't' && fgetc(f) == '=')
        {
            char * tmp = get_value(f);
            if (!tmp)
                continue;
            else if (strlen(tmp) < MAX_INIT_PATH_SIZE)
                strcpy(cmdline.init, tmp);
            free(tmp);
        }
        else if (c == 'r' && fgetc(f) == 'o' && fgetc(f) == 'o' && fgetc(f) == 't' && fgetc(f) == '=')
            cmdline.root = get_value(f);
        else if (c == 'f' && fgetc(f) == 's' && fgetc(f) == '=')
            cmdline.fs = get_value(f);
    }
    fclose(f);
    return cmdline;
}
