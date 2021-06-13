ENTRY(_start)

SECTIONS
{
    /* for initial physical address space*/
    . = 0x40000000;
    .text : {
        *(.boot.text);
        *(.text);
    }
    .data : {
        *(.data)
    }
    .rodata : {
        _module_init_start = .;
        *(.module_init);
        _module_init_end = .;
    }
    _bss_start =.;
    .bss : {
        *(.bss)
        . = ALIGN(8);
        . = . + 0x1000;
        _stack_top = .;

        . = ALIGN(8);
        _guest_stack_end = .;
        . += 0x1000;
        _guest_stack_top = .;
        . += 0x1000;
        _kstack_for_guest = .;
    }
    _bss_end = .;
}
