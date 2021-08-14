#include <cgreen/cgreen.h>
#include <core/fdt.h>
#include <stdlib.h>

Describe (FDT);
BeforeEach (FDT)
{
}
AfterEach (FDT)
{
}

const char *qemu_dtb_filename = "data/qemu_virt.dtb";
const char *test_dtb_0 = "data/test_00.dtb";
const char *test_dtb_1 = "data/test_01.dtb";

struct fdt_header *get_dtb_base (const char *filename);

Ensure (FDT, get_address)
{
	void *fdt = get_dtb_base (qemu_dtb_filename);
	assert_that (fdt != NULL);
}

struct fdt *fdt_setup_struct (struct fdt_header *);
Ensure (FDT, setup_struct)
{
	struct fdt *fdt = fdt_setup_struct (
		(struct fdt_header *)get_dtb_base (qemu_dtb_filename));
	assert_that (fdt != NULL);
}

struct fdt *fdt_parse (struct fdt_header *);
Ensure (FDT, fdt_parse)
{
	struct fdt *fdt = fdt_parse (
		(struct fdt_header *)get_dtb_base (qemu_dtb_filename));
	assert_that (fdt != NULL);
}

/* generated by `$fdtdump data/qemu_virt.dtb`
/dts-v1/;
// magic:               0xd00dfeed
// totalsize:           0x4f1 (1265)
// off_dt_struct:       0x38
// off_dt_strings:      0x408
// off_mem_rsvmap:      0x28
// version:             17
// last_comp_version:   16
// boot_cpuid_phys:     0x0
// size_dt_strings:     0xe9
// size_dt_struct:      0x3d0
...
*/

uint32_t fdt_calc_structblock_totalsize (struct fdt_node *);
Ensure (FDT, structblock_size_all)
{
	struct fdt *fdt = fdt_parse (
		(struct fdt_header *)get_dtb_base (qemu_dtb_filename));
	assert_that (fdt != NULL);
	assert_that (NULL != fdt->root_node);

	int size = fdt_calc_structblock_totalsize (fdt->root_node);
	printf ("totalsize %d\n", size);
	// the value 0x3d0 is obtained by fdtdump cmd.
	assert_that (size == 0x3d0);
}

/*
$ fdtdump test_00.dtb

**** fdtdump is a low-level debugging tool, not meant for general use.
**** If you want to decompile a dtb, you probably want
****     dtc -I dtb -O dts <filename>

/dts-v1/;
// magic:               0xd00dfeed
// totalsize:           0x48 (72)
// off_dt_struct:       0x38
// off_dt_strings:      0x48
// off_mem_rsvmap:      0x28
// version:             17
// last_comp_version:   16
// boot_cpuid_phys:     0x0
// size_dt_strings:     0x0
// size_dt_struct:      0x10

/ {
};
 */
Ensure (FDT, structblock_size_0)
{
	struct fdt *fdt =
		fdt_parse ((struct fdt_header *)get_dtb_base (test_dtb_0));
	assert_that (fdt != NULL);
	assert_that (NULL != fdt->root_node);

	int size = fdt_calc_structblock_totalsize (fdt->root_node);
	printf ("totalsize %d\n", size);
	assert_that (size == 0x10);
}
/*
$ fdtdump test_01.dtb

**** fdtdump is a low-level debugging tool, not meant for general use.
**** If you want to decompile a dtb, you probably want
****     dtc -I dtb -O dts <filename>

/dts-v1/;
// magic:               0xd00dfeed
// totalsize:           0xcf (207)
// off_dt_struct:       0x38
// off_dt_strings:      0x98
// off_mem_rsvmap:      0x28
// version:             17
// last_comp_version:   16
// boot_cpuid_phys:     0x0
// size_dt_strings:     0x37
// size_dt_struct:      0x60

/ {
    interrupt-parent = <0x00008001>;
    #size-cells = <0x00000002>;
    #address-cells = <0x00000002>;
    compatible = "linux,dummy-virt";
};
 */
Ensure (FDT, structblock_size_1)
{
	struct fdt *fdt =
		fdt_parse ((struct fdt_header *)get_dtb_base (test_dtb_1));
	assert_that (fdt != NULL);
	assert_that (NULL != fdt->root_node);

	int size = fdt_calc_structblock_totalsize (fdt->root_node);
	printf ("totalsize %d\n", size);
	assert_that (size == 0x60);
}

Ensure (FDT, stringblock_size)
{
}

int
main (int argc, char **argv)
{
	TestSuite *suite = create_test_suite ();
	add_test_with_context (suite, FDT, get_address);
	add_test_with_context (suite, FDT, setup_struct);
	add_test_with_context (suite, FDT, structblock_size_0);
	add_test_with_context (suite, FDT, structblock_size_1);
	add_test_with_context (suite, FDT, structblock_size_all);
	add_test_with_context (suite, FDT, stringblock_size);
	return run_test_suite (suite, create_text_reporter ());
}