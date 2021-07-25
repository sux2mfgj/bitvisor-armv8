#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"

#define KEYWORD_MAINMENU "mainmenu"
#define KEYWORD_MENU "menu"
#define KEYWORD_SRC "source"

#define NOT_YET_IMPLEMENTED \
	errx (1, "not yet implemented (%s:%s:%d)", __FILE__, __func__, __LINE__)

const char *kw_type_str[] = {
	[KW_MAINMENU] = "mainmenu", [KW_MENU] = "menu",
	[KW_ENDMENU] = "endmenu",   [KW_SOURCE] = "source",
	[KW_CONFIG] = "config",
};

static struct kconfig_base *_parse_kconfig (struct kconfig *kcfg);

static bool
has_newline (char *text)
{
	return text[strlen (text) - 1] == '\n';
}

// trim_newline - this function has side effect.
static char *
trim_newline (char *text)
{
	if (has_newline (text))
		text[strlen (text) - 1] = '\0';

	return text;
}

static struct kconfig_base *
parse_mainmenu (struct kconfig *kcfg, char *kw, char *remain)
{
	kconfig_mm_t *mm = calloc (1, sizeof (kconfig_mm_t));
	struct kconfig_base *base = &mm->base;
	base->kw = kw;
	base->type = KW_MAINMENU;
	mm->title = trim_newline (remain);
	warnx ("main menu : %s", mm->title);
	return (struct kconfig_base *)mm;
}

static struct kconfig_base *
parse_menu (struct kconfig *kcfg, char *kw, char *remain)
{
	kconfig_menu_t *menu = calloc (1, sizeof (kconfig_menu_t));
	struct kconfig_base *base = &menu->base;
	base->kw = kw;
	base->type = KW_MENU;

	menu->title = trim_newline (remain);

	struct kconfig_base *entry;
	struct kconfig_base **current = &menu->head;
	do {
		entry = _parse_kconfig (kcfg);
		if (!entry)
			errx (1, "endmenu is not found");
		(*current) = entry;
		current = &(*current)->next;

		CONF_DEBUG ("menu: %s", kw_type_str[entry->type]);
	} while (entry->type != KW_ENDMENU);

	return (struct kconfig_base *)menu;
}

static struct kconfig_base *
parse_source (struct kconfig *kcfg, char *kw, char *remain)
{
	NOT_YET_IMPLEMENTED;
	return NULL;
}

static struct kconfig_base *
parse_dummy (struct kconfig *kcfg, char *kw, char *remain)
{
	CONF_DEBUG ("%s kw: %s, rem: %s", __func__, kw, trim_newline (remain));
	NOT_YET_IMPLEMENTED;
	return NULL;
}

static struct kconfig_base *
parse_endmenu (struct kconfig *kcfg, char *kw, char *remain)
{
	struct kconfig_base *base = calloc (1, sizeof (struct kconfig_base));
	base->kw = kw;
	base->type = KW_ENDMENU;

	strtok (NULL, " ");
	CONF_DEBUG ("%s endmenu", kw);

	return base;
}

/* clang-format off */ // TODO find suitable configuration
static struct parse_pair root_parse_pair[] = {
    { "mainmenu", parse_mainmenu, },
    { "menu", parse_menu, },
    { "source", parse_source },
    { "config", parse_dummy },
    { "endmenu", parse_endmenu },
    { "" } };

static struct kconfig_base *
_parse_kconfig (struct kconfig *kcfg)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

retry:
	nread = getline (&line, &len, kcfg->file);
	if (nread == -1) {
		// reach to end of line
		if (errno == 0)
			return NULL;
		else
			err (1, "failed to read config file");
	}
	if (nread == 1 && line[0] == '\n') {
		goto retry;
	}

	// The strtok func has side effect. be careful.
	char *kw = trim_newline (strtok (line, " "));
	char *rem;
	if (has_newline (kw)) {
		kw = trim_newline (kw);
		rem = NULL;
	} else {
		rem = kw + strlen (kw) + 2; // 2 to skip null filled by strtok.
	}

	struct kconfig_base *base;
	for (struct parse_pair *pp = root_parse_pair; pp->word[0]; pp++) {
		if (!strcmp (pp->word, kw)) {
			base = pp->parse (kcfg, kw, rem);
			if (!base)
				errx (1, "failed to parse the %s", pp->word);

			return base;
		}
	}

	errx (1, "detects unknown word(%s)", kw);

	return NULL;
}

struct kconfig *
parse_kconfig (char const *filename)
{
	struct kconfig *kconfig;
	FILE *file;

	file = fopen (filename, "r");
	if (!file) {
		warn ("Cannot open the file (%s)", filename);
		goto err0;
	}

	kconfig = calloc (1, sizeof *kconfig);
	if (!kconfig) {
		warn ("failed to allocate memory");
		goto err1;
	}

	kconfig->filename = filename;
	kconfig->file = file;

	struct kconfig_base *base;
	struct kconfig_base **current = &kconfig->head;
	do {
		base = _parse_kconfig (kconfig);
		if (!base)
			break;

		*current = base;
		current = &((*current)->next);
	} while (base);

	return kconfig;
err1:
	// TODO close file
err0:
	return NULL;
}
