
#include <asm.h>
#include <core/initfunc.h>
#include <core/types.h>
#include <core/vcpu.h>

#include "context.h"
#include "mmu.h"
#include "tcr.h"

#define MMU_4K_INDEX_MASK (0b111111111)
#define MMU_4K_L1_INDEX_OFFSET 30
#define MMU_4K_L2_INDEX_OFFSET 21
#define MMU_4K_L3_INDEX_OFFSET 12

extern u8 head[], end[];

static u64 *
page_walk (u64 *root_pt, int level, u64 phys)
{
	u16 index;
	u64 *tt = root_pt;
	u64 *tt_entry = NULL;

	u16 tt_index_offsets[] = {
		MMU_4K_L1_INDEX_OFFSET,
		MMU_4K_L2_INDEX_OFFSET,
		MMU_4K_L3_INDEX_OFFSET,
	};

	ASSERT (level <= 3);

	for (int i = 0; level; level--, i++) {
		index = (phys >> tt_index_offsets[i]) & MMU_4K_INDEX_MASK;
		tt_entry = &tt[index];

		if (!*tt_entry & PTE_PRESENT) {
			u64 *tmp_tt, tmp_tt_phys;
			alloc_pages ((void **)&tmp_tt, &tmp_tt_phys, 2);
			*tt_entry = tmp_tt_phys | PTE_PRESENT;
		}

		tt = tt_entry;
	}

	return tt_entry;
}

static void *
aarch64_map_1gpage (u64 *root_pt, u64 gphys, u32 flags)
{
	ASSERT (!(gphys & PAGESIZE1G_MASK));
	u64 *tt_entry = page_walk (root_pt, 1, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	// TODO use the flags
	*tt_entry = gphys | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;
	return (void *)gphys;
}

static void *
aarch64_map_2mpage (u64 *root_pt, u64 gphys, u32 flags)
{
	ASSERT (!(gphys & PAGESIZE2M_MASK));
	u64 *tt_entry = page_walk (root_pt, 2, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	// TODO use the flags
	*tt_entry = gphys | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;
	return (void *)gphys;
}

static void *
aarch64_map_page (u64 *root_pt, u64 gphys, u32 flags)
{
	ASSERT (!(gphys & PAGESIZE_MASK));
	u64 *tt_entry = page_walk (root_pt, 3, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	// TODO use the flags
	*tt_entry = gphys | PTE_PAGE | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;

	return (void *)gphys;
}

void *
vm_mapmem (u64 physaddr, uint len, u32 flags)
{
	// TODO consider to use `at` instruction.
	u64 *root_tt = (u64 *)read_ttbr0_el2 ();
	void *virt;

	switch (len) {
	case PAGESIZE:
		virt = aarch64_map_page (root_tt, physaddr, flags);
		break;
	case PAGESIZE2M:
		virt = aarch64_map_2mpage (root_tt, physaddr, flags);
		break;
	case PAGESIZE1G:
		virt = aarch64_map_1gpage (root_tt, physaddr, flags);
		break;
	default:
		panic ("%s: unsupported length (0x%x)", __func__, len);
	}

	return virt;
}

void
vm_unmapmem (void *ptr, uint len)
{
	not_yet_implemented ();
}

/*
 * translation fault handler.
 */
void
aarch64_guest_tfault_handler (u64 gphys)
{
	struct aarch64_vm_ctx *ctx = (struct aarch64_vm_ctx *)current->vm_ctx;

	// TODO mmio lock

	/*
	 * Try to access to mmio range first, because, the page fault for
	 * straight page map occures only once for each region, but mmio for
	 * devices occurs multiple times.
	 */
	// TODO
	if (mmio_access_page (gphys, true))
		goto done;

	if (!mmio_range (gphys & ~PAGESIZE1G_MASK, PAGESIZE1G))
		aarch64_map_1gpage (ctx->vttbr_el2, gphys, 0);
	else if (!mmio_range (gphys & ~PAGESIZE2M_MASK, PAGESIZE2M))
		aarch64_map_2mpage (ctx->vttbr_el2, gphys, 0);
	else
		aarch64_map_page (ctx->vttbr_el2, gphys, 0);

done:;
	// TODO mmio unlock
}

bool
arch_flush_tlb_entry (phys_t gpst, phys_t gpend)
{
	// TODO
	return true;
}

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
	l2_pt_virt[l2_index] = (u64)l3_pt_phys | PTE_TABLE | PTE_PRESENT;
	l2_pt_virt[0] = 0x00000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
			PTE_PRESENT | LOW_ATTR_AF;

	// TODO
	for (i = 0; i < 512; ++i) {
		if (i > 440 && i < 460)
			printf ("%d 0x%x 0x%llx\n", i, i,
				(0x40000000ULL + (i * l3_size)));
		l3_pt_virt[i] = (0x40000000ULL + (i * l3_size)) | PTE_BLOCK |
				PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT |
				LOW_ATTR_AF;
	}

	// for (i = 0; i < VMMSIZE_ALL >> PAGESIZE2M_SHIFT; i++) {
	//	ASSERT (l3_index + i < 512);
	//	phys_t base = head & -0x200000 + (i * l3_size);
	//	phys_t base = (phys_t)head + (i * l3_size);
	//	l3_pt_virt[l3_index + i] = base | PTE_BLOCK |
	//				   PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT |
	//				   LOW_ATTR_AF;
	//}

	// disable mmu
	u64 sctlr_el2 = read_sctlr_el2 ();
	sctlr_el2 &= ~0x1; // SCTLR_EL2_MMU_EN
	write_sctlr_el2 (sctlr_el2);

	// save to ttbr0
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
