
#include <core/types.h>
#include <core/vcpu.h>

#include "context.h"
#include "mmu.h"

#define MMU_4K_INDEX_MASK (0b111111111)
#define MMU_4K_L1_INDEX_OFFSET 30
#define MMU_4K_L2_INDEX_OFFSET 21
#define MMU_4K_L3_INDEX_OFFSET 12

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
			alloc_page ((void **)&tmp_tt, &tmp_tt_phys);
			*tt_entry = tmp_tt_phys | PTE_PRESENT;
		}

		tt = tt_entry;
	}

	return tt_entry;
}

static void
aarch64_map_guest_1gpage (u64 *root_pt, u64 gphys)
{
	ASSERT (!(gphys & PAGESIZE1G_MASK));
	u64 *tt_entry = page_walk (root_pt, 1, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	*tt_entry = gphys | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;
}

static void
aarch64_map_guest_2mpage (u64 *root_pt, u64 gphys)
{
	ASSERT (!(gphys & PAGESIZE2M_MASK));
	u64 *tt_entry = page_walk (root_pt, 2, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	*tt_entry = gphys | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;
}

static void
aarch64_map_guest_page (u64 *root_pt, u64 gphys)
{
	ASSERT (!(gphys & PAGESIZE_MASK));
	u64 *tt_entry = page_walk (root_pt, 3, gphys);
	if (!tt_entry)
		panic ("failed page walking");

	*tt_entry = gphys | PTE_PAGE | PTE_MRA_DEVICE_nGnRnE | LOW_ATTR_AF |
		    PTE_PRESENT | MM_MEM_ATTR | MM_STAGE2_SH |
		    MM_STAGE2_S2AP_RW;
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
	 * devices occurs multiple time.
	 */
	// TODO
	// if (mmio_access_page(gphys, true))
	//    goto done;

	if (!mmio_range (gphys & ~PAGESIZE1G_MASK, PAGESIZE1G))
		aarch64_map_guest_1gpage (ctx->vttbr_el2, gphys);
	else if (!mmio_range (gphys & ~PAGESIZE2M_MASK, PAGESIZE2M))
		aarch64_map_guest_2mpage (ctx->vttbr_el2, gphys);
	else
		aarch64_map_guest_page (ctx->vttbr_el2, gphys);

	// done:
	// TODO mmio unlock
}

bool
arch_flush_tlb_entry (phys_t gpst, phys_t gpend)
{
	// TODO
	return true;
}
