#ifndef _DRV_GIC_H
#define _DRV_GIC_H

enum {
	FDT_REG_INDEX_GIC_GICD, // distributer interface
	FDT_REG_INDEX_GIC_GICC, // cpu interface
	FDT_REG_INDEX_GIC_GICH, // hypervisor interface
	FDT_REG_INDEX_GIC_GICV, // virtual cpu interface
};

#endif
