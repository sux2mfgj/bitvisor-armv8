#include <core.h>
#include <core/fdt.h>
#include <core/initfunc.h>
#include <core/int.h>
#include <core/mm.h>
#include <core/panic.h>
#include <driver/gic.h>
#include <driver/vgic.h>

// CPU Interface register map
enum {
	GICC_CTLR = 0x0000,
	GICC_PMR = 0x0004,
	GICC_BPR = 0x0008,
	GICC_IAR = 0x000c,
	GICC_EOIR = 0x0010,
	GICC_RPR = 0x0014,
	GICC_HPPIR = 0x0018,
	GICC_ABPR = 0x001c,
	GICC_AIAR = 0x0020,
	GICC_AEOIR = 0x0024,
	GICC_AHPPIR = 0x0028,
	GICC_APR = 0x00d0,
	GICC_NSAPR = 0x00e0,
	GICC_IIDR = 0x00fc,
	GICC_DIR = 0x1000,
};

// Distributor register map
enum {
	GICD_CTLR = 0x000,
	GICD_TYPER = 0x004,
	GICD_IIDR = 0x008,
	GICD_IGPOUPR = 0x80,
	GICD_ISENABLER = 0x100,
	GICD_ICENABLER = 0x180,
	GICD_ISPENDR = 0x200,
	GICD_ICPENDR = 0x280,
	GICD_ISACTIVER = 0x300,
	GICD_ICACTIVER = 0x380,
	GICD_IPRIORITYR = 0x400,
	GICD_ITARGETSR = 0x800,
	GICD_ICFGR = 0xc00,
	GICD_NSACR = 0xe00,
	GICD_SGIR = 0xf00,
	GICD_CPENDSGIR = 0xf10,
	GICD_SPENDSGIR = 0xf20,
};

struct gic_dev {
	u64 cpu_mmio_base;
	u64 dist_mmio_base;
};

static u32
gicc_read_32 (struct gic_dev *gic, size_t offset)
{
	return *(volatile u32 *)(gic->cpu_mmio_base + offset);
}

static void
gicc_write_32 (struct gic_dev *gic, size_t offset, u32 value)
{
	*(volatile u32 *)(gic->cpu_mmio_base + offset) = value;
}

static u32
gicd_read_32 (struct gic_dev *gic, size_t offset)
{
	return *(volatile u32 *)(gic->dist_mmio_base + offset);
}

static void
gicd_write_32 (struct gic_dev *gic, size_t offset, u32 value)
{
	*(volatile u32 *)(gic->dist_mmio_base + offset) = value;
}

void
gic_init_controller (struct gic_dev *gic)
{
	// stop distributer
	gicd_write_32 (gic, GICD_CTLR, 0);

	volatile u32 iidr = gicc_read_32 (gic, GICC_IIDR);

	// initialize CPU interface controller //TODO remove magic number
	gicc_write_32 (gic, GICC_CTLR, 0b1000000001);

	// set to lowest priority
	gicc_write_32 (gic, GICC_PMR, 0xff);

	// setup priority group
	gicc_write_32 (gic, GICC_BPR, 0x0);

	// enable distributer
	gicd_write_32 (gic, GICD_CTLR, 1);
}

// TODO setup
static struct irqc gic_irqc = {
	.name = "Arm Generic Interrupt Controller (GIC)"
	//.priv;
	//.eof =
};

static void
gic_fdt_init (struct fdt_node *node)
{
	int res;
	u64 gicd_base, gicc_base;

	res = fdt_get_reg_value (node, FDT_REG_INDEX_GIC_GICD, FDT_REG_ADDRESS,
				 &gicd_base);
	if (res) {
		panic ("oh no");
	}

	res = fdt_get_reg_value (node, FDT_REG_INDEX_GIC_GICC, FDT_REG_ADDRESS,
				 &gicc_base);
	if (res) {
		panic ("oh no");
	}

	struct gic_dev *gic = alloc (sizeof (struct gic_dev));
	gic->cpu_mmio_base = gicc_base;
	gic->dist_mmio_base = gicd_base;

	gic_init_controller (gic);

	vgic_init (node);

	int_register_irqc (&gic_irqc);
}

static struct fdt_driver gic_fdt_driver = {
	.compatible = "arm,cortex-a15-gic",
	.init = gic_fdt_init,
};

FDT_DRIVER (gic, &gic_fdt_driver);
