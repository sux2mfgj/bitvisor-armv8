#include <asm.h>
#include <core.h>
#include <core/mm.h>
#include <core/panic.h>
#include <core/pcpu.h>
#include <core/string.h>
#include <core/vcpu.h>
#include <core/virt.h>
#include <core/vmctl.h>

#include "context.h"
#include "mmu.h"
#include "system.h"

struct aarch64_vm_ctx {
	void *vttbr_el2;
	u64 vtcr_el2;
	struct aarch64_context *guest_ctx;
	struct aarch64_context *host_ctx;
} __attribute__ ((packed));

// defined in vecotr.S
void _context_switch (struct aarch64_context *prev,
		      struct aarch64_context *next);

#define ESR_EL2_ISS_OFFSET 0
#define ESR_EL2_ISS_MASK (0x00ffffff)
#define ESR_EL2_EL_OFFSET 25
#define ESR_EL2_EL_MASK (0b1)
#define ESR_EL2_EC_OFFSET 26
#define ESR_EL2_EC_MASK 0b111111
#define ESR_EL2_ISS2_OFFSET 32
#define ESR_EL2_ISS2_MASK 0b11111

enum esr_exception_class {
	EC_UNKNOWN_REASON = 0b000000,
	EC_TRAPPED_WF = 0b000001,
	EC_INST_ABORT_FORM_LOW_EL = 0b100000,
	EC_DATA_ABORT_FROM_LOW_EL = 0b100100,
	EC_TRAPPED_MSR_MRS = 0b011000
};

static void
handle_guest_exit (void)
{
	u64 esr = read_esr_el2 ();

	u32 ec = (esr >> ESR_EL2_EC_OFFSET) & ESR_EL2_EC_MASK;
	u32 iss = (esr & ESR_EL2_ISS_MASK);
	u32 il = (esr >> ESR_EL2_IL_OFFSET) & ESR_EL2_IL_OFFSET;

	switch (ec) {
	case EC_UNKNOWN_REASON:
		panic ("found unknown exception code (1)");
	case EC_TRAPPED_WF:
	case EC_INST_ABORT_FORM_LOW_EL:
	case EC_DATA_ABORT_FROM_LOW_EL:
	case EC_TRAPPED_MSR_MRS:
		panic ("not yet implemented %s:%d", __func__, __LINE__);
	default:
		panic ("found unknown exception code (2)");
	};
}

static void
vm_mainloop (void)
{
	struct aarch64_vm_ctx *vm_ctx =
		(struct aarch64_vm_ctx *)current->vm_ctx;
	for (;;) {
		// schedule ();

		_context_switch (vm_ctx->host_ctx, vm_ctx->guest_ctx);
		handle_guest_exit ();

		// TODO
		panic ("not yet implemented (%s:%s:%d)", __FILE__, __func__,
		       __LINE__);
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

	struct aarch64_context *guest_ctx =
		(struct aarch64_context *)alloc (sizeof *ctx->guest_ctx);
	guest_ctx->elr = (u64)0x50000000; // TODO
	guest_ctx->spsr = SPSR_EL2_M_EL1H;
	memset (guest_ctx->regs, 0x0, sizeof guest_ctx->regs);
	guest_ctx->regs[0] = 0x40000000; // TODO fdt base address

	ctx->guest_ctx = guest_ctx;
	ctx->host_ctx = (struct aarch64_context *)alloc (sizeof *ctx->host_ctx);

	current->vm_ctx = ctx;
}

static void
arch64_vminit (void)
{
	struct aarch64_vm_ctx *ctx = alloc (sizeof (struct aarch64_vm_ctx));
	current->vm_ctx = ctx;

	aarch64_paging_init ();

	u64 tmp;
#define SPSEL_ELX 0b1
	tmp = SPSEL_ELX;
	write_spsel (tmp);

	alloc_page ((void **)&tmp, NULL);
	write_sp_el2 (tmp + PAGESIZE);
}

// Hypervisoe Configuration Register (HCR_EL2)
#define HCR_EL2_VM_OFFSET 0
#define HCR_EL2_VM_ENABLE (0b1 << HCR_EL2_VM_OFFSET)

#define HCR_EL2_SWIO_OFFSET 1

#define HCR_EL2_PTW_OFFSET 2

#define HCR_EL2_FMO_OFFSET 3
#define HCR_EL2_FMO (0b1 << HCR_EL2_FMO_OFFSET)

#define HCR_EL2_IMO_OFFSET 4
#define HCR_EL2_IMO (0b1 << HCR_EL2_IMO_OFFSET)

#define HCR_EL2_AMO_OFFSET 5
#define HCR_EL2_AMO (0b1 << HCR_EL2_AMO_OFFSET)

#define HCR_EL2_VF_OFFSET 6

#define HCR_EL2_VI_OFFSET 7
#define HCR_EL2_VI (0b1 << HCR_EL2_VI_OFFSET)

#define HCR_EL2_VSE_OFFSET 8

#define HCR_EL2_FB_OFFSET 9

#define HCR_EL2_BSU_OFFSET 10

#define HCR_EL2_TWI_OFFSET (13)
#define HCR_EL2_TWI (0b1 << HCR_EL2_TWI_OFFSET)

#define HCR_EL2_RW_OFFSET 31
#define HCR_EL2_AARCH32 (0b0ULL << HCR_EL2_RW_OFFSET)
#define HCR_EL2_AARCH64 (0b1ULL << HCR_EL2_RW_OFFSET)
// TODO

// Armv8.1
#define HCR_EL2_E2H_OFFSET (34)
#define HCR_EL2_E2H (0b1 << HCR_EL2_E2H_OFFSET)

static void
setup_hcr_el2 (void)
{
	u64 hcr_el2 = read_hcr_el2 ();

	hcr_el2 |= HCR_EL2_AARCH64 | HCR_EL2_TWI | HCR_EL2_VM_ENABLE;

	write_hcr_el2 (hcr_el2);
}

static void
arch64_startvm (void)
{
	setup_hcr_el2 ();
	vm_mainloop ();
}

static struct vmctl_func func = {
	.vminit = arch64_vminit,
	.start_vm = arch64_startvm,
};

int
arch_check_virt_available (void)
{
	return FULLVIRTUALIZE_ARMV8_2;
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
