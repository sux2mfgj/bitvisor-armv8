#include <asm.h>
#include <core/tls.h>

struct tls*
get_tls (void)
{
	return (struct tls*)read_tpidr_el2 ();
}

void
set_tls (struct tls* tls)
{
	write_tpidr_el2 ((u64)tls);
}
