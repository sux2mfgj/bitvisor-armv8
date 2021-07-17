
#ifndef __CORE_INT_H
#define __CORE_INT_H

struct irqc {
	char *name;
	// TODO add other functions.
	void (*eoi) (int irq);
};

// global irq controller
extern struct irqc *girqc;

void int_register_irqc (struct irqc *irqc);

#endif
