#include <core/panic.h>
#include <irq_defs.h>
#include <asm.h>

#include "context.h"
#include "exception.h"

static void elx_sync_handler(struct aarch64_context *ctx)
{
    u64 esr = read_esr_el2();
}

void
aarch64_irq_handler (struct aarch64_context *ctx, u64 irq_type)
{
	switch (irq_type) {
        case IRQ_VEC_ELX_SYNC:
            elx_sync_handler(ctx);
            break;;
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
		not_yet_implemented();
	}
}
