
#include "printf.h"
#include "types.h"

/* print a message and stop */
void __attribute__ ((noreturn)) panic (char *format, ...)
{
	va_list ap;
	va_start (ap, format);
	vprintf (format, ap);
	va_end (ap);

	// TODO
	while (true)
		asm volatile("wfi");
}

void
panic_test (void)
{
	// TODO
}
