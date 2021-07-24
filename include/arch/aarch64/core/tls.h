#ifndef _TLS_H_
#define _TLS_H_

#include <core/pcpu.h>
#include <core/vcpu.h>

// thread local storage
struct tls {
	struct pcpu* pcpu;
	struct vcpu* vcpu;
};

struct tls* get_tls (void);
void set_tls (struct tls* tls);

#endif // _TLS_H_
