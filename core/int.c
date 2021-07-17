
#include "int.h"

#include "asm.h"
#include "initfunc.h"
#include "printf.h"

extern void irq_vector (void);

struct irqc *girqc;

void
int_register_irqc (struct irqc *irqc)
{
	if (girqc) {
		printf ("global irq controller is overwrided: %s -> %s",
			girqc->name, irqc->name);
	}

	// TODO check handlers
	girqc = irqc;
}

// TODO fix the arugment
void
irq_handler (void)
{
}

// int callfunc_and_getint (asmlinkage void (*func)(void *arg), void *arg);
// int do_externalint_enable (void);
// void int_exceptionHandler (int intnum, void *handler);
// void set_int_handler (int intnum, void *handler);
// void int_init_ap (void);

static void
int_init_global (void)
{
	write_vbar_el2 ((u64)irq_vector);
	// TODO remove magic number
	daif_clear (2);

	// u64 hcr_el2 = read_hcr_el2();
	// hcr_el2 |=
}

INITFUNC ("global1", int_init_global);

