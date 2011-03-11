#include "cmdline.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT stdout

static bool
is_blank(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == EOF);
}

static char *
get_value(FILE * f) {
	char * path = (char *) malloc(11*sizeof(char)); /* /sbin/init size */
	int i = 0;
	char c;
	while (!is_blank(c = fgetc(f)))
	{
		path[i] = c;
		if (++i%11 == 0)
			path = (char *) realloc(path, (i+11) * sizeof(char));
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
	fprintf(OUTPUT, "Parsing kernel cmdline for root and init paths...\n");
	Cmdline paths;
	paths.root = NULL;
	paths.init[0] = '\0';
	FILE * f = fopen("/proc/cmdline", "r");
	if (!f)
		return paths;
	char c;
	while ((c = fgetc(f)) != EOF)
	{
		if (c == 'i' && fgetc(f) == 'n' && fgetc(f) == 'i' && fgetc(f) == 't' && fgetc(f) == '=')
		{
			char * tmp = get_value(f);
			if (!tmp)
				continue;
			else if (strlen(tmp) < MAX_INIT_PATH_SIZE)
				strcpy(paths.init, tmp);
			free(tmp);
		}
		else if (c == 'r' && fgetc(f) == 'o' && fgetc(f) == 'o' && fgetc(f) == 't' && fgetc(f) == '=')
			paths.root = get_value(f);
	}
	fclose(f);
	return paths;
}
