#include <core/fdt.h>
#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct fdt_driver *__init_1_start[0], *__init_1_end[0];

// dummy to link
void *
get_fdt_base (void)
{
	return NULL;
}

static void *dtb_data = NULL;
struct fdt_header *
get_dtb_base (const char *filename)
{
	struct stat stbuf;
	if (dtb_data)
		return dtb_data;

	// const char *filename = "data/qemu_virt.dtb";
	int dtb_fd = open (filename, O_RDONLY);
	if (dtb_fd < 0) {
		warn ("failed to opne the file : %s", filename);
		return NULL;
	}

	if (fstat (dtb_fd, &stbuf) == -1) {
		warn ("failed to get fstat");
		return NULL;
	}
	int len = stbuf.st_size;

	dtb_data = calloc (1, len);
	if (!dtb_data)
		return NULL;

	if (read (dtb_fd, dtb_data, len) != len) {
		return NULL;
	}

	close (dtb_fd);

	// in qemu-user-static, mmap doesn't work correctly.
	// dtb_map = mmap (NULL, stbuf.st_size, PROT_READ, MAP_SHARED, dtb_fd,
	// 0); if (MAP_FAILED == dtb_map) { 	warn ("failed the mmap");
	// return
	// NULL;
	//}

	return dtb_data;
}

int
alloc_pages (void **virt, u64 *phys, int n)
{
	*virt = calloc (n, 0x1000);
	//*virt = malloc (n * PAGESIZE);
	return 0;
}

void *
alloc (uint len)
{
	return calloc (1, len);
}

void
panic (char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	vprintf (fmt, ap);
	va_end (ap);

	errx (1, "panic");
}
