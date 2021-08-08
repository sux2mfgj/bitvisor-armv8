
#include <core/types.h>
#include <core/vcpu.h>

#include "context.h"

static void
aarch64_map_guest_1gpage (u64 *root_pt, u64 gphys)
{
}

static void
aarch64_map_guest_2mpage (u64 *root_pt, u64 gphys)
{
}

static void
aarch64_map_guest_page (u64 *root_pt, u64 gphys)
{
}

void
aarch64_guest_page_fault (u64 gphys)
{
	struct aarch64_vm_ctx *ctx = (struct aarch64_vm_ctx *)current->vm_ctx;

	ctx->vttbr_el2;

	// TODO mmio lock

	/*
	 * Try to access to mmio range first, because, the page fault for
	 * straight page map happens only once for each region, but mmio for
	 * devices happens multiple time.
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

done:
	// TODO mmio unlock
}

bool
arch_flush_tlb_entry (phys_t gpst, phys_t gpend)
{
	// TODO
	return true;
}
