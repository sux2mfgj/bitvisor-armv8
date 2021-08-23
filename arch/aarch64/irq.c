#include <asm.h>
#include <core/panic.h>
#include <core/printf.h>
#include <irq_defs.h>

#include "context.h"
#include "exception.h"

static void
handle_data_abort_same_el(struct aarch64_context *ctx)
{
    u64 elr = read_elr_el2 ();
    printf ("0x%llx\n", elr);
    not_yet_implemented();
}

static void
elx_sync_handler (struct aarch64_context *ctx)
{
	u64 esr = read_esr_el2 ();

	u32 ec = (esr >> ESR_EL2_EC_OFFSET) & ESR_EL2_EC_MASK;
	u32 iss = (esr & ESR_EL2_ISS_MASK);
	// u32 il = (esr >> ESR_EL2_IL_OFFSET) & ESR_EL2_IL_OFFSET;

	switch ((enum esr_exception_class)ec) {
	case EC_UNKNOWN_REASON:
	case EC_TRAPPED_WF:
	case EC_TRAPPED_MCR_MRC_F:
	case EC_TRAPPED_MCRR_MRRC:
	case EC_TRAPPED_MCR_MRC_E:
    case EC_DATA_ABORT_SAME_EL:
        handle_data_abort_same_el(ctx);
        break;
    default:
		panic ("not yet implemented ec %d\n", ec);
	}

	not_yet_implemented ();
}

void
aarch64_irq_handler (struct aarch64_context *ctx, u64 irq_type)
{
	switch (irq_type) {
	case IRQ_VEC_ELX_SYNC:
		elx_sync_handler (ctx);
		break;
	case IRQ_VEC_EL1_SYNC:
	case IRQ_VEC_EL1_IRQ:
	case IRQ_VEC_EL1_FIQ:
	case IRQ_VEC_EL1_SERR:
	case IRQ_VEC_ELX_IRQ:
	case IRQ_VEC_ELX_FIQ:
	case IRQ_VEC_ELX_SERR:
	case IRQ_VEC_EL0_64_SYNC:
	case IRQ_VEC_EL0_64_IRQ:
	case IRQ_VEC_EL0_64_FIQ:
	case IRQ_VEC_EL0_64_SERR:
	case IRQ_VEC_EL0_32_SYNC:
	case IRQ_VEC_EL0_32_IRQ:
	case IRQ_VEC_EL0_32_FIQ:
	case IRQ_VEC_EL0_32_SERR:
	default:
		not_yet_implemented ();
	}
}
