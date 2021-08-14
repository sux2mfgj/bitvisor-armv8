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

void *get_fdt_base (void);
Ensure (FDT, get_address)
{
	void *fdt = get_fdt_base ();
	assert_that (fdt != NULL);
}

Ensure (FDT, setup_struct)
{
	struct fdt *fdt_setup_struct (struct fdt_header *);
	struct fdt *fdt =
		fdt_setup_struct ((struct fdt_header *)get_fdt_base ());
	assert_that (fdt != NULL);
}

int
main (int argc, char **argv)
{
	TestSuite *suite = create_test_suite ();
	add_test_with_context (suite, FDT, get_address);
	add_test_with_context (suite, FDT, setup_struct);
	return run_test_suite (suite, create_text_reporter ());
}
