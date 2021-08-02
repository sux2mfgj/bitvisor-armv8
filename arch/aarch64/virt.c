#include <core.h>
#include <core/mm.h>
#include <core/pcpu.h>
#include <core/string.h>
#include <core/vcpu.h>
#include <core/virt.h>
#include <core/vmctl.h>
#include <asm.h>

#include "mmu.h"

struct aarch64_vm_ctx {
	void *vttbr_el2;
	u64 vtcr_el2;
};

static void
vm_mainloop (void)
{
	for (;;) {
		// schedule ();
	}
}

static void
aarch64_paging_init (void)
{
	u64 *st2_l1_pt;
	struct aarch64_vm_ctx *ctx = (struct aarch64_vm_ctx *)current->vm_ctx;

	alloc_page ((void **)&st2_l1_pt, NULL);
	memset (st2_l1_pt, 0x0, PAGESIZE);

	ctx->vttbr_el2 = st2_l1_pt;

	ctx->vtcr_el2 = VTCR_EL2_T0SZ | VTCR_EL2_IRGN0_NC | VTCR_EL2_ORGN0_NC |
		       VTCR_EL2_TG0_4K | VTCR_EL2_PS_40 | VTCR_EL2_SH0_NS |
		       VTCR_EL2_NSW_NS | VTCR_EL2_NSA_NS | VTCR_EL2_SL0;

	write_vttbr_el2 ((u64)ctx->vttbr_el2);
	write_vtcr_el2 ((u64)ctx->vtcr_el2);
	isb ();
}

static void
arch64_vminit (void)
{
	struct aarch64_vm_ctx *ctx = alloc (sizeof (struct aarch64_vm_ctx));
	current->vm_ctx = ctx;

	aarch64_paging_init ();
}

static void
arch64_startvm (void)
{
	vm_mainloop ();
}

static struct vmctl_func func = {
	.vminit = arch64_vminit,
	.start_vm = arch64_startvm,
};

int
arch_check_virt_available (void)
{
	// TODO
	return 0;
}

int
arch_virt_init (void)
{
	return FULLVIRTUALIZE_ARMV8_2;
}

void
arch_vmctl_init (void)
{
	memcpy ((void *)&current->vmctl, (void *)&func, sizeof func);
}
