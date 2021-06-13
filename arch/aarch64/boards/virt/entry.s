.section .entry

.globl entry
entry:
    // checking current exception level. it must be EL2.
    mrs x0, CurrentEL
    and x0, x0, #0b1100
    cmp x0, #0b1000
    b.ne unknown_el

    ldr x30, =start_stack
    mov sp, x30

    //adr x9, l0_page_table
    //msr ttbr0_el2, x9
    ////TODO in my environment, following error is occured
    ////boot.S:20: Error: selected processor does not support system register name 'ttbr1_el2'
    ////msr ttbr1_el2, x9

    //ldr x9, =TCR_VALUE
    //msr tcr_el2, x9

    //ldr x9, =MAIR_VALUE
    //msr mair_el2, x9

    //isb
    //mrs x9, sctlr_el2
    //orr x9, x9, #SCTLR_EL2_MMU_EN
    //msr sctlr_el2, x9

    //isb

    bl vmm_main
    b .

.globl unknown_el
unknown_el:
    b .

    # Stack for initialization
    # TODO use macro
    .align 12
    .space 0x1000
start_stack:
