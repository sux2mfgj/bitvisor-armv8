
#include <core/tls.h>

struct vcpu *
get_current_arch ()
{
	struct tls *tls = get_tls ();

	return tls->vcpu;
}
