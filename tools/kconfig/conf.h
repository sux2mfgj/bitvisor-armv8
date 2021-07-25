#ifndef _CONF_H_
#define _CONF_H_

#include <stdio.h>

struct kconfig {
	const char* filename;
    FILE* file;
};

struct kconfig* parse_kconfig (const char* filename);

#endif // _CONF_H_
