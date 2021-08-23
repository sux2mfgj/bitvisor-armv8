
#include <core.h>

#include "asm.h"
#include "assert.h"
#include "constants.h"
#include "initfunc.h"
#include "mm.h"
#include "tcr.h"
#include "types.h"

#define PTE_PRESENT (1 << 0)
#define PTE_TABLE (1 << 1)
#define PTE_PAGE (1 << 1)
#define PTE_BLOCK (0 << 1)

// memory region attributes
#define MAIR_DEVICE_nGnRnE 0x00
#define MAIR_NORMAL 0xff
#define MAIR_DEVICE_POS 0
#define MAIR_NORMAL_POS 1
#define MAIR_VALUE                                       \
	((MAIR_DEVICE_nGnRnE << (8 * MAIR_DEVICE_POS)) | \
	 (MAIR_NORMAL << (8 * MAIR_NORMAL_POS)))

#define PTE_MRA_OFFSET 2
#define PTE_MRA_DEVICE_nGnRnE (MAIR_DEVICE_POS << PTE_MRA_OFFSET)
#define PTE_MRA_DEVICE PTE_MRA_DEVICE_nGnRnE
#define PTE_MRA_NORMAL (MAIR_DEVICE_POS << PTE_MRA_OFFSET)

// lower attributes
#define LOW_ATTR_ATTR_IDX_OFFSET (2)
#define LOW_ATTR_NS (1 << 5)
#define LOW_ATTR_AP_OFFSET (6)
#define LOW_ATTR_SH_OFFSET (8)
#define LOW_ATTR_AF (1 << 10)
#define LOW_ATTR_NG (1 << 11)
#define LOW_ATTR_OA_OFFSET (12)
#define LOW_ATTR_NT (1 << 16)

extern u8 head[], end[];

// static u64 vmm_l1_pt[512] __attribute__ ((aligned (PAGESIZE)));
// static u64 vmm_l2_pt[512] __attribute__ ((aligned (PAGESIZE)));

static void
vm_init_global (void)
{
	int i;

	int l3_size = PAGESIZE * 512;
	int l2_size = l3_size * 512;
	int l1_size = l2_size * 512;

	int l1_index = ((phys_t)head / l1_size) & (512 - 1);
	int l2_index = ((phys_t)head / l2_size) & (512 - 1);
	int l3_index = ((phys_t)head / l3_size) & (512 - 1);

	u64 *l1_pt_virt, l1_pt_phys;
	u64 *l2_pt_virt, l2_pt_phys;
	u64 *l3_pt_virt, l3_pt_phys;

	alloc_pages ((void **)&l1_pt_virt, &l1_pt_phys, 2);
	alloc_pages ((void **)&l2_pt_virt, &l2_pt_phys, 2);
	alloc_pages ((void **)&l3_pt_virt, &l3_pt_phys, 2);
	memset (l1_pt_virt, 0x00, PAGESIZE * 2);
	memset (l2_pt_virt, 0x00, PAGESIZE * 2);
	memset (l3_pt_virt, 0x00, PAGESIZE * 2);

	printf ("0x%llx 0x%llx 0x%llx\n", (u64)l1_pt_phys, (u64)l2_pt_phys,
		(u64)l3_pt_phys);
	printf ("l1 0x%x, l2 0x%x l3 0x%x\n", l1_index, l2_index, l3_index);

	l1_pt_virt[l1_index] = (u64)l2_pt_phys | PTE_TABLE | PTE_PRESENT;
	// l2_pt_virt[l2_index] = (u64)l3_pt_phys | PTE_TABLE | PTE_PRESENT;
	l2_pt_virt[0] = 0x00000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
			PTE_PRESENT | LOW_ATTR_AF;
	l2_pt_virt[1] = 0x40000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
			PTE_PRESENT | LOW_ATTR_AF;

	// for (i = 0; i < 512; ++i) {
	//	if (i > 440 && i < 460)
	//		printf ("%d 0x%x 0x%llx\n", i, i,
	//			(0x40000000ULL + (i * l3_size)));
	//	l3_pt_virt[i] = (0x40000000ULL + (i * l3_size)) | PTE_BLOCK |
	//			PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT |
	//			LOW_ATTR_AF;
	//}

	// for (i = 0; i < VMMSIZE_ALL >> PAGESIZE2M_SHIFT; i++) {
	//	ASSERT (l3_index + i < 512);
	//	phys_t base = head & -0x200000 + (i * l3_size);
	//	phys_t base = (phys_t)head + (i * l3_size);
	//	l3_pt_virt[l3_index + i] = base | PTE_BLOCK |
	//				   PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT |
	//				   LOW_ATTR_AF;
	//}

	// vmm_l1_pt[0] = (u64)vmm_l2_pt | PTE_TABLE | PTE_PRESENT;
	// vmm_l2_pt[0] = 0x00000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
	//	       PTE_PRESENT | LOW_ATTR_AF;
	// vmm_l2_pt[1] = 0x40000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
	//	       PTE_PRESENT | LOW_ATTR_AF;
	// vmm_l2_pt[2] = 0x80000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
	//	       PTE_PRESENT | LOW_ATTR_AF;

	// disable mmu
	u64 sctlr_el2 = read_sctlr_el2 ();
	sctlr_el2 &= ~0x1; // SCTLR_EL2_MMU_EN
	write_sctlr_el2 (sctlr_el2);

	// save to ttbr0
	// write_ttbr0_el2 ((u64)vmm_l1_pt);
	write_ttbr0_el2 ((u64)l1_pt_phys);

	// setup TCR
	write_tcr_el2 ((u64)TCR_VALUE);

	// setup mair
	write_mair_el2 ((u64)MAIR_VALUE);

	// call isb
	isb ();

	// enable mmu (sctlr_el2)
	sctlr_el2 = read_sctlr_el2 ();
	sctlr_el2 |= 0x1; // SCTLR_EL2_MMU_EN
	write_sctlr_el2 (sctlr_el2);

	// call isb
	isb ();
}

// vm_init_global depends on the mm subsystem. It is initialized "global2".
INITFUNC ("global3", vm_init_global);
