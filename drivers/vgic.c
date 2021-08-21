#include <core.h>
#include <core/fdt.h>
#include <core/mm.h>
#include <core/printf.h>
#include <driver/gic.h>

enum {
	GICH_HCR = 0x000,
	GICH_VTR = 0x004,
	GICH_VMCR = 0x008,
	GICH_MISR = 0x010,
	GICH_EISR0 = 0x020,
	GICH_EISR1 = 0x024,
	GICH_ELSR0 = 0x030,
	GICH_ELSR1 = 0x034,
	GICH_APR = 0x0f0,
	GICH_APR1 = 0x0f4,
	GICH_APR2 = 0x0f8,
	GICH_APR3 = 0x0fc,
	GICH_LR_BASE = 0x100,
	GICH_LR_LAST = 0x1fc,
};

#define DEF_OFFSET_MASK_32(name, offset, mask)                                \
	static const u32 name##_OFFSET __attribute__ ((__unused__)) = offset; \
	static const u32 name##_MASK __attribute__ ((__unused__)) =           \
		((1 << (mask + 1)) - 1);
#define EXTRACT_VALUE(value, offset, mask) ((value >> offset) & mask)

DEF_OFFSET_MASK_32 (GICH_HCR_EN, 0, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_UIE, 1, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_LRENPIE, 2, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_NPIE, 3, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_VGRP0EIE, 4, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_VGRP0DIE, 5, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_VGRP1EIE, 6, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_VGRP1DIE, 7, 1)
DEF_OFFSET_MASK_32 (GICH_HCR_EOICOUNT, 27, 4)

DEF_OFFSET_MASK_32 (GICH_VTR_LIST_REG, 0, 6)
DEF_OFFSET_MASK_32 (GICH_VTR_PRE, 26, 3)
DEF_OFFSET_MASK_32 (GICH_VTR_PRI, 29, 3)

DEF_OFFSET_MASK_32 (GICH_LR_VID, 0, 10)
DEF_OFFSET_MASK_32 (GICH_LR_PID, 10, 10)
DEF_OFFSET_MASK_32 (GICH_LR_PRIORITY, 23, 5)
DEF_OFFSET_MASK_32 (GICH_LR_STATE, 28, 2)
DEF_OFFSET_MASK_32 (GICH_LR_GROUP, 30, 1);
DEF_OFFSET_MASK_32 (GICH_LR_HW, 31, 1);

enum vgic_interrupt_state {
	VGIC_INT_STATE_INVALID = 0b00,
	VGIC_INT_STATE_PENDING = 0b01,
	VGIC_INT_STATE_ACTIVE = 0b10,
	VGIC_INT_STATE_PEND_ACT = 0b11,
};

struct vgic_dev {
	u64 virt_mmio_base;
	u64 hyp_mmio_base;
};

inline static u32
gicv_read_32 (struct vgic_dev *dev, int offset)
{
	return *(volatile u32 *)(dev->virt_mmio_base + offset);
}

inline static void
gicv_write_32 (struct vgic_dev *dev, int offset, u32 value)
{
	*(volatile u32 *)(dev->virt_mmio_base + offset) = value;
}

inline static u32
gich_read_32 (struct vgic_dev *dev, int offset)
{
	return *(volatile u32 *)(dev->hyp_mmio_base + offset);
}

inline static void
gich_write_32 (struct vgic_dev *dev, int offset, u32 value)
{
	*(volatile u32 *)(dev->hyp_mmio_base + offset) = value;
}

static void
vgic_disable (struct vgic_dev *dev)
{
	u32 hcr = gich_read_32 (dev, GICH_HCR);

	hcr &= ~(GICH_HCR_EN_MASK << GICH_HCR_EN_OFFSET);
	gich_write_32 (dev, GICH_HCR, hcr);
}

static void
vgic_enable (struct vgic_dev *dev)
{
	u32 hcr = gich_read_32 (dev, GICH_HCR);

	hcr |= GICH_HCR_EN_MASK << GICH_HCR_EN_OFFSET;
	gich_write_32 (dev, GICH_HCR, hcr);
}

static int
vgic_get_n_listregs (struct vgic_dev *dev)
{
	u32 vtr = gich_read_32 (dev, GICH_VTR);

	// According to a gic spec, a ListRegs indicates list register s minus
	// one.
	return EXTRACT_VALUE (vtr, GICH_VTR_LIST_REG_OFFSET,
			      GICH_VTR_LIST_REG_MASK) +
	       1;
}

static void
vgic_setup_list_register (struct vgic_dev *dev, int lr_index, u16 vid, u16 pid,
			  u8 priority, enum vgic_interrupt_state state,
			  u8 group, bool hw)
{
	int offset = GICH_LR_BASE + (lr_index * sizeof (u32));

	u32 value = (u32)vid | (u32)pid << GICH_LR_PID_OFFSET |
		    (u32)priority << GICH_LR_PRIORITY_OFFSET |
		    (u32)state << GICH_LR_STATE_OFFSET |
		    (u32)group << GICH_LR_GROUP_OFFSET |
		    (u32)(hw ? 0b1 : 0b0) << GICH_LR_HW_OFFSET;

	gich_write_32 (dev, offset, value);
}

void
vgic_init (struct fdt_node *node)
{
	int ret;
	u64 gich_base, gicv_base;

	ret = fdt_get_reg_value (node, FDT_REG_INDEX_GIC_GICH, FDT_REG_ADDRESS,
				 &gich_base);
	if (ret)
		panic ("oh no");

	ret = fdt_get_reg_value (node, FDT_REG_INDEX_GIC_GICV, FDT_REG_ADDRESS,
				 &gicv_base);
	if (ret)
		panic ("oh no");

    printf ("initalize vgic: gich 0x%llx, gicv 0x%llx\n", gich_base, gicv_base);

	struct vgic_dev *dev = alloc (sizeof (struct vgic_dev));
	dev->virt_mmio_base = gicv_base;
	dev->hyp_mmio_base = gich_base;

	vgic_disable (dev);

	int nlistregs = vgic_get_n_listregs (dev);

	for (int i = 0; i < nlistregs; ++i) {
		vgic_setup_list_register (dev, 0, 0, 0, 0,
					  VGIC_INT_STATE_INVALID, 0, false);
	}

	vgic_enable (dev);
}
