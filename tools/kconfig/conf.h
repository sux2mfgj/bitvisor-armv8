#ifndef _CONF_H_
#define _CONF_H_

#include <err.h>
#include <stdio.h>

// TODO fix a case that output string only.
#define CONF_DEBUG(fmt, ...) \
	warnx ("debug(%s:%d): " fmt, __func__, __LINE__, __VA_ARGS__)

struct kconfig_base;

struct kconfig {
	const char *filename;
	FILE *file;
	struct kconfig_base *head;
};

enum kw_type {
	KW_MAINMENU,
	KW_MENU,
	KW_ENDMENU,
	KW_SOURCE,
	KW_CONFIG,
};

extern const char *kw_type_str[];

struct kconfig_base {
	char *kw;
	enum kw_type type;
	struct kconfig_base *next;
};

struct kconfig_text {
	struct kconfig_base base;
	char *title;
};

typedef struct kconfig_menu {
	struct kconfig_base base;
	char *title;
	// entries
	struct kconfig_base *head;
} kconfig_menu_t;

typedef struct kconfig_text kconfig_mm_t;

struct parse_pair {
	char *word;
	struct kconfig_base *(*parse) (struct kconfig *kcfg, char *kw,
				       char *remain);
};

struct kconfig *parse_kconfig (const char *filename);

#endif // _CONF_H_
