#include "conf.h"

#include <err.h>

int
main (int argc, char const *argv[])
{
	char const *filename = "Kconfig";

	if (argc == 2) {
		filename = argv[1];
	}
	warnx ("%s is selected", filename);

	struct kconfig *kconfig = parse_kconfig (filename);
	if (!kconfig)
		return 1;

	warnx ("success");

	return 0;
}
