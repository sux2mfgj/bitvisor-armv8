#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <core/fdt.h>

struct fdt_driver *__init_1_start[0], *__init_1_end[0];

static int dtb_fd = 0;
static void *dtb_data = NULL;
void *
get_fdt_base (void)
{
	struct stat stbuf;
	if (dtb_data)
		return dtb_data;

	if (!dtb_fd) {
		const char *filename = "data/qemu_virt.dtb";
		dtb_fd = open (filename, O_RDONLY);
		if (dtb_fd < 0) {
			warn ("failed to opne the file : %s", filename);
			return NULL;
		}
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

	// in qemu-user-static, mmap doesn't work correctly.
	// dtb_map = mmap (NULL, stbuf.st_size, PROT_READ, MAP_SHARED, dtb_fd,
	// 0); if (MAP_FAILED == dtb_map) { 	warn ("failed the mmap"); 	return
	//NULL;
	//}

	return dtb_data;
}

int
alloc_pages (void **virt, u64 *phys, int n)
{
	*virt = malloc (n * 0x1000);
	//*virt = malloc (n * PAGESIZE);
}

int *
alloc (uint len)
{
	return malloc (len);
}
