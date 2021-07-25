#include <err.h>
#include <stdlib.h>

#include "conf.h"

#define KEYWORD_MAINMENU "mainmenu"
#define KEYWORD_MENU "menu"
#define KEYWORD_SRC "source"

static const char *root_keywords[] = {
    "mainmenu",
    "menu",
    "source",
    "config",
    ""
};

static int
_parse_kconfig(struct kconfig* cfg)
{
    char *line = NULL;
    size_t len;
    size_t ret;

    ret = getline(&line, &len, cfg->file);
    if (ret < 0) {
        warn("failed to read config file");
        return -1;
    }

    for(char **kw = root_keywords; *kw[0]; kw++) {
        warnx("%s", *kw);
    }

    return 0;
}

struct kconfig*
parse_kconfig (const char* filename)
{
	struct kconfig* kconfig;
	FILE *file;

	file = fopen (filename, "r");
	if (!file) {
		warn ("Cannot open the file (%s)", filename);
		goto err0;
	}

    kconfig = calloc(1, sizeof *kconfig);
    if (!kconfig) {
        warn ("failed to allocate memory");
        goto err1;
    }

    kconfig->filename = filename;
    kconfig->file = file;


    int ret;
    ret = _parse_kconfig(kconfig);
    if (ret)
        goto err1;

    errx(1, "not yet implemented");
err1:
    //TODO close file
err0:
	return NULL;
}
