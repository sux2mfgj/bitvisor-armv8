#include "irq_defs.h"
#include "../../context.h"

#define TRAPFRAME_SIZE 264

// void _context_switch(struct aarch64_context *prev,
//                      struct aarch64_context *next)
.globl _context_switch
_context_switch:
    // x0 is a pointer to prev, x1 is to next

    // save registers
    stp x0, x1, [x0, #CTX_OFFSET_REG_0]
    stp x2, x3, [x0, #CTX_OFFSET_REG_2]
    stp x4, x5, [x0, #CTX_OFFSET_REG_4]
    stp x6, x7, [x0, #CTX_OFFSET_REG_6]
    stp x8, x9, [x0, #CTX_OFFSET_REG_8]
    stp x10, x11, [x0, #CTX_OFFSET_REG_10]
    stp x12, x13, [x0, #CTX_OFFSET_REG_12]
    stp x14, x15, [x0, #CTX_OFFSET_REG_14]
    stp x16, x17, [x0, #CTX_OFFSET_REG_16]
    stp x18, x19, [x0, #CTX_OFFSET_REG_18]
    stp x20, x21, [x0, #CTX_OFFSET_REG_20]
    stp x22, x23, [x0, #CTX_OFFSET_REG_22]
    stp x24, x25, [x0, #CTX_OFFSET_REG_24]
    stp x26, x27, [x0, #CTX_OFFSET_REG_26]
    stp x28, x29, [x0, #CTX_OFFSET_REG_28]

    mrs x2, elr_el2
    stp x30, x2, [x0, #CTX_OFFSET_REG_30]

    mrs x2, spsr_el2
    mov x3, sp
    stp x2, x3, [x0, #CTX_OFFSET_SPSR]

    // restore registers
    ldp x2, x3, [x1, #CTX_OFFSET_REG_2]
    ldp x4, x5, [x1, #CTX_OFFSET_REG_4]
    ldp x6, x7, [x1, #CTX_OFFSET_REG_6]
    ldp x8, x9, [x1, #CTX_OFFSET_REG_8]
    ldp x10, x11, [x1, #CTX_OFFSET_REG_10]
    ldp x12, x13, [x1, #CTX_OFFSET_REG_12]
    ldp x14, x15, [x1, #CTX_OFFSET_REG_14]
    ldp x16, x17, [x1, #CTX_OFFSET_REG_16]
    ldp x18, x19, [x1, #CTX_OFFSET_REG_18]
    ldp x20, x21, [x1, #CTX_OFFSET_REG_20]
    ldp x22, x23, [x1, #CTX_OFFSET_REG_22]
    ldp x24, x25, [x1, #CTX_OFFSET_REG_24]
    ldp x26, x27, [x1, #CTX_OFFSET_REG_26]
    ldp x28, x29, [x1, #CTX_OFFSET_REG_28]

    ldp x0, x30, [x1, #CTX_OFFSET_SPSR]
    msr spsr_el2, x0
    mov sp, x30

    ldp x0, x30, [x1, #CTX_OFFSET_REG_30]
    msr elr_el2, x0

    ldp x0, x1, [x1, #CTX_OFFSET_REG_0]

    eret

// TODO remove magic numbers. almost offset of struct members.
.globl switch_from_guest
switch_from_guest:
    // save guest x0, x1 regs to stack temporarly
    sub sp, sp, #16
    ldp x0, x1, [sp, #0]

    // load `struct tls *` variable
    mrs x0, tpidr_el2
    // load tls->vcpu (current)
    ldr x1, [x0, #8]
    // load vcpu->vm_ctx (struct aarch64_vm_ctx*)
    ldr x0, [x1, #8]
    // load vmctx->guest_ctx (struct context*)
    ldr x1, [x0, #16]

    // save guest context
    stp x2, x3, [x1, #CTX_OFFSET_REG_2]
    stp x4, x5, [x1, #CTX_OFFSET_REG_4]
    stp x6, x7, [x1, #CTX_OFFSET_REG_6]
    stp x8, x9, [x1, #CTX_OFFSET_REG_8]
    stp x10, x11, [x1, #CTX_OFFSET_REG_10]
    stp x12, x13, [x1, #CTX_OFFSET_REG_12]
    stp x14, x15, [x1, #CTX_OFFSET_REG_14]
    stp x16, x17, [x1, #CTX_OFFSET_REG_16]
    stp x18, x19, [x1, #CTX_OFFSET_REG_18]
    stp x20, x21, [x1, #CTX_OFFSET_REG_20]
    stp x22, x23, [x1, #CTX_OFFSET_REG_22]
    stp x24, x25, [x1, #CTX_OFFSET_REG_24]
    stp x26, x27, [x1, #CTX_OFFSET_REG_26]
    stp x28, x29, [x1, #CTX_OFFSET_REG_28]

    mrs x2, elr_el2
    stp x30, x2, [x1, #CTX_OFFSET_REG_30]

    mrs x2, spsr_el2
    mrs x3, sp_el1
    //mov x3, sp
    stp x2, x3, [x1, #CTX_OFFSET_SPSR]

    // restore host context
    // load vmctx->host_ctx (struct context*)
    ldr x1, [x0, #24]

    // restore sp
    ldr x2, [x1, #CTX_OFFSET_SP]
    mov sp, x2

    ldr x30, [x1, #CTX_OFFSET_REG_30]

    ldp x2, x3, [x1, #CTX_OFFSET_SPSR]
    msr elr_el2, x2
    msr spsr_el2, x3

    // restore registers
    ldp x2, x3, [x1, #CTX_OFFSET_REG_2]
    ldp x4, x5, [x1, #CTX_OFFSET_REG_4]
    ldp x6, x7, [x1, #CTX_OFFSET_REG_6]
    ldp x8, x9, [x1, #CTX_OFFSET_REG_8]
    ldp x10, x11, [x1, #CTX_OFFSET_REG_10]
    ldp x12, x13, [x1, #CTX_OFFSET_REG_12]
    ldp x14, x15, [x1, #CTX_OFFSET_REG_14]
    ldp x16, x17, [x1, #CTX_OFFSET_REG_16]
    ldp x18, x19, [x1, #CTX_OFFSET_REG_18]
    ldp x20, x21, [x1, #CTX_OFFSET_REG_20]
    ldp x22, x23, [x1, #CTX_OFFSET_REG_22]
    ldp x24, x25, [x1, #CTX_OFFSET_REG_24]
    ldp x26, x27, [x1, #CTX_OFFSET_REG_26]
    ldp x28, x29, [x1, #CTX_OFFSET_REG_28]
    ldr x30, [x1, #CTX_OFFSET_REG_30]

    ldp x0, x1, [x1, #CTX_OFFSET_REG_0]

    ret

.globl return_from_exception
return_from_exception:
    // store context(registers) ,and return to a cause of exception address.
    // restore link register and elr
    ldp x30, x0, [sp, #CTX_OFFSET_REG_30]
    msr elr_el2, x0

    // restore processor state register
    ldp x0, x1, [sp, #CTX_OFFSET_SPSR]
    msr spsr_el2, x0

    // restore general purpose registers
    ldp x0, x1, [sp, #CTX_OFFSET_REG_0]
    ldp x2, x3, [sp, #CTX_OFFSET_REG_2]
    ldp x4, x5, [sp, #CTX_OFFSET_REG_4]
    ldp x6, x7, [sp, #CTX_OFFSET_REG_6]
    ldp x8, x9, [sp, #CTX_OFFSET_REG_8]
    ldp x10, x11, [sp, #CTX_OFFSET_REG_10]
    ldp x12, x13, [sp, #CTX_OFFSET_REG_12]
    ldp x14, x15, [sp, #CTX_OFFSET_REG_14]
    ldp x16, x17, [sp, #CTX_OFFSET_REG_16]
    ldp x18, x19, [sp, #CTX_OFFSET_REG_18]
    ldp x20, x21, [sp, #CTX_OFFSET_REG_20]
    ldp x22, x23, [sp, #CTX_OFFSET_REG_22]
    ldp x24, x25, [sp, #CTX_OFFSET_REG_24]
    ldp x26, x27, [sp, #CTX_OFFSET_REG_26]
    ldp x28, x29, [sp, #CTX_OFFSET_REG_28]
    ldr x30, [sp, #CTX_OFFSET_REG_30]

    // restore stack pointer
    add sp, sp, #TRAPFRAME_SIZE
    eret

.macro RUN_HANDLER type
    .align 7
    save_regs_\type:
    // allocate space in stack
    sub sp, sp, #TRAPFRAME_SIZE
    // save general purpose registers
    stp x0, x1, [sp, #CTX_OFFSET_REG_0]
    stp x2, x3, [sp, #CTX_OFFSET_REG_2]
    stp x4, x5, [sp, #CTX_OFFSET_REG_4]
    stp x6, x7, [sp, #CTX_OFFSET_REG_6]
    stp x8, x9, [sp, #CTX_OFFSET_REG_8]
    stp x10, x11, [sp, #CTX_OFFSET_REG_10]
    stp x12, x13, [sp, #CTX_OFFSET_REG_12]
    stp x14, x15, [sp, #CTX_OFFSET_REG_14]
    stp x16, x17, [sp, #CTX_OFFSET_REG_16]
    stp x18, x19, [sp, #CTX_OFFSET_REG_18]
    stp x20, x21, [sp, #CTX_OFFSET_REG_20]
    stp x22, x23, [sp, #CTX_OFFSET_REG_22]
    stp x24, x25, [sp, #CTX_OFFSET_REG_24]
    stp x26, x27, [sp, #CTX_OFFSET_REG_26]
    stp x28, x29, [sp, #CTX_OFFSET_REG_28]

        // save exception link register
    mrs x0, elr_el2
    // save link register
    stp x30, x0, [sp, #CTX_OFFSET_REG_30]

    // save processor state register
    mrs x1, spsr_el2
    mov x0, sp
    stp x0, x1, [sp, #CTX_OFFSET_SPSR]

    // setup argument to pass a generated trapframe
    mov x0, sp
    mov x1, #\type
    // call a trap handler written in c
    bl aarch64_irq_handler
    b return_from_exception

.endm

.section .text

.globl irq_vector

// alignment
.align 11
irq_vector:
el1_sp0:
    RUN_HANDLER IRQ_VEC_EL1_SYNC
    RUN_HANDLER IRQ_VEC_EL1_IRQ
    RUN_HANDLER IRQ_VEC_EL1_FIQ
    RUN_HANDLER IRQ_VEC_EL1_SERR
el1_spx:
    RUN_HANDLER IRQ_VEC_ELX_SYNC
    RUN_HANDLER IRQ_VEC_ELX_IRQ
    RUN_HANDLER IRQ_VEC_ELX_FIQ
    RUN_HANDLER IRQ_VEC_ELX_SERR
el0_aarch64:
    .align 7
    b switch_from_guest
    RUN_HANDLER IRQ_VEC_EL0_64_IRQ
    RUN_HANDLER IRQ_VEC_EL0_64_FIQ
    RUN_HANDLER IRQ_VEC_EL0_64_SERR
el0_aarch32:
    RUN_HANDLER IRQ_VEC_EL0_32_SYNC
    RUN_HANDLER IRQ_VEC_EL0_32_IRQ
    RUN_HANDLER IRQ_VEC_EL0_32_FIQ
    RUN_HANDLER IRQ_VEC_EL0_32_SERR
