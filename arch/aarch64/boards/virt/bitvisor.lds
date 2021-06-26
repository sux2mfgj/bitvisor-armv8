ENTRY(entry)

phys_base = 0x78000000;

SECTIONS {
    . = phys_base;
    head = .;
    .text : {
		text = .;
		*(.entry)
		*(.text)
		codeend = .;
	}
    .data : {
		data = .;
		*(.data)
		. = ALIGN (8);
        __initfunc_start = .;
        *(.initfunc)
        __initfunc_end = .;
		__process_builtin = .;
		*(.processes)
		__process_builtin_end = .;
		dataend = .;
	}
    .bss : {
		bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN (4096);
	}
	end = .;
}
