#include "irq_defs.h"

#define TRAPFRAME_SIZE 264

// void _context_switch(struct aarch64_context *prev,
//                      struct aarch64_context *next)
.globl _context_switch
_context_switch:
    // x0 is a pointer to prev, x1 is to next

    // save registers
    stp x0, x1, [x0, #16 * 0]
    stp x2, x3, [x0, #16 * 1]
    stp x4, x5, [x0, #16 * 2]
    stp x6, x7, [x0, #16 * 3]
    stp x8, x9, [x0, #16 * 4]
    stp x10, x11, [x0, #16 * 5]
    stp x12, x13, [x0, #16 * 6]
    stp x14, x15, [x0, #16 * 7]
    stp x16, x17, [x0, #16 * 8]
    stp x18, x19, [x0, #16 * 9]
    stp x20, x21, [x0, #16 * 10]
    stp x22, x23, [x0, #16 * 11]
    stp x24, x25, [x0, #16 * 12]
    stp x26, x27, [x0, #16 * 13]
    stp x28, x29, [x0, #16 * 14]

    mrs x2, elr_el2
    stp x30, x2, [x0, #16 * 15]

    mrs x2, spsr_el2
    mov x3, sp
    stp x2, x3, [x0, #16 * 16]

    // restore registers
    ldp x2, x3, [x1, #16 * 1]
    ldp x4, x5, [x1, #16 * 2]
    ldp x6, x7, [x1, #16 * 3]
    ldp x8, x9, [x1, #16 * 4]
    ldp x10, x11, [x1, #16 * 5]
    ldp x12, x13, [x1, #16 * 6]
    ldp x14, x15, [x1, #16 * 7]
    ldp x16, x17, [x1, #16 * 8]
    ldp x18, x19, [x1, #16 * 9]
    ldp x20, x21, [x1, #16 * 10]
    ldp x22, x23, [x1, #16 * 11]
    ldp x24, x25, [x1, #16 * 12]
    ldp x26, x27, [x1, #16 * 13]
    ldp x28, x29, [x1, #16 * 14]

    ldp x0, x30, [x1, #16 * 16]
    msr spsr_el2, x0
    mov sp, x30

    ldp x0, x30, [x1, #16 * 15]
    msr elr_el2, x0

    ldp x0, x1, [x1, #16 * 0]

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
    stp x2, x3, [x1, #16 * 1]
    stp x4, x5, [x1, #16 * 2]
    stp x6, x7, [x1, #16 * 3]
    stp x8, x9, [x1, #16 * 4]
    stp x10, x11, [x1, #16 * 5]
    stp x12, x13, [x1, #16 * 6]
    stp x14, x15, [x1, #16 * 7]
    stp x16, x17, [x1, #16 * 8]
    stp x18, x19, [x1, #16 * 9]
    stp x20, x21, [x1, #16 * 10]
    stp x22, x23, [x1, #16 * 11]
    stp x24, x25, [x1, #16 * 12]
    stp x26, x27, [x1, #16 * 13]
    stp x28, x29, [x1, #16 * 14]

    mrs x2, elr_el2
    stp x30, x2, [x1, #16 * 15]

    mrs x2, spsr_el2
    mov x3, sp
    stp x2, x3, [x1, #16 * 16]

    // push stacked guest x0 and x1 regs
    stp x2, x3, [sp, #0]
    stp x2, x3, [x1, #16 * 0]
    add sp, sp, #16

    // restore host context
    // load vmctx->host_ctx (struct context*)
    ldr x1, [x0, #24]

    // load x0 and x1 and push to stack
    ldp x2, x3, [x1, #16 * 0]
    sub sp, sp, #16
    stp x2, x3, [sp, #0]

    ldr x30, [x1, #16 * 16]

    ldp x2, x3, [x1, #16 * 15]
    msr elr_el2, x2
    msr spsr_el2, x3

    ldp x28, x29, [x1, #16 * 14]
    ldp x26, x27, [x1, #16 * 13]
    ldp x24, x25, [x1, #16 * 12]
    ldp x22, x23, [x1, #16 * 11]
    ldp x20, x21, [x1, #16 * 10]
    ldp x18, x19, [x1, #16 * 9]
    ldp x16, x17, [x1, #16 * 8]
    ldp x14, x15, [x1, #16 * 7]
    ldp x12, x13, [x1, #16 * 6]
    ldp x10, x11, [x1, #16 * 5]
    ldp x8, x9, [x1, #16 * 4]
    ldp x6, x7, [x1, #16 * 3]
    ldp x4, x5, [x1, #16 * 2]
    ldp x2, x3, [x1, #16 * 1]

    // restore saved x0 and x1 from stack
    ldp x0, x1, [sp, #0]
    add sp, sp, #16

    ret

.globl return_from_exception
return_from_exception:
    // store context(registers) ,and return to a cause of exception address.
    // restore link register
    ldr x30, [sp, #16 * 16]
    // restore exception link register
    ldp x0, x1, [sp, #16 * 15]
    msr elr_el2, x0
    // restore processor state register
    msr spsr_el2, x1

    // restore general purpose registers
    ldp x28, x29, [sp, #16 * 14]
    ldp x26, x27, [sp, #16 * 13]
    ldp x24, x25, [sp, #16 * 12]
    ldp x22, x23, [sp, #16 * 11]
    ldp x20, x21, [sp, #16 * 10]
    ldp x18, x19, [sp, #16 * 9]
    ldp x16, x17, [sp, #16 * 8]
    ldp x14, x15, [sp, #16 * 7]
    ldp x12, x13, [sp, #16 * 6]
    ldp x10, x11, [sp, #16 * 5]
    ldp x8, x9, [sp, #16 * 4]
    ldp x6, x7, [sp, #16 * 3]
    ldp x4, x5, [sp, #16 * 2]
    ldp x2, x3, [sp, #16 * 1]
    ldp x0, x1, [sp, #16 * 0]

    // restore stack pointer
    add sp, sp, #TRAPFRAME_SIZE
    eret

.macro RUN_HANDLER type
    .align 7
    save_regs_\type:
    // allocate space in stack
    sub sp, sp, #TRAPFRAME_SIZE
    // save general purpose registers
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    // save exception link register
    mrs x0, elr_el2
    // save processor state register
    mrs x1, spsr_el2
    stp x0, x1, [sp, #16 * 15]
    // save link register
    str x30, [sp, #16 * 16]
    // setup argument to pass a generated trapframe
    mov x0, sp

    mov x1, #\type
    // call a trap handler written in c
    bl irq_handler
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
