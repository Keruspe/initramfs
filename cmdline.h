#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include <stdlib.h>

#define MAX_INIT_PATH_SIZE 25

typedef struct {
	char init[MAX_INIT_PATH_SIZE+1]; /* Or we'll have a leak */
	char * root;
} Cmdline;

Cmdline parse_kernel_cmdline();

#endif /* __CMDLINE_H__ */
