#include <asm.h>
#include <core/initfunc.h>
#include <core/pcpu.h>
#include <core/string.h>

// thread local storage
struct tls {
	struct pcpu *pcpu;
};

static struct pcpu boot_pcpu;

struct tls *
get_tls (void)
{
	return (struct tls *)read_tpidr_el2 ();
}

struct pcpu *
get_currentcpu_arch (void)
{
	struct tls *tls = get_tls ();
	return tls->pcpu;
}

static void
pcpu_init_global (void)
{
	memcpy (&boot_pcpu, &pcpu_default, sizeof (struct pcpu));
	boot_pcpu.cpunum = 0;

	struct tls *tls = get_tls ();
	tls->pcpu = &boot_pcpu;

	pcpu_init ();
	pcpu_list_add (&boot_pcpu);
}

INITFUNC ("global0", pcpu_init_global);
