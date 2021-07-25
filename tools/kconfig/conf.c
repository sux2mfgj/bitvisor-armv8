#include <err.h>
#include "conf.h"

int main(int argc, char const* argv[])
{
    char *filename = "Kconfig";

    if (argc == 2) {
        filename = argv[1];
    }

    struct kconfig *kconfig = parse_kconfig(filename);
    if (!kconfig) {
        errx(1, "failed to parse the kconfig");
    }

    return 0;
}
