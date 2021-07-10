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

        __init_start = .;
        __init_0_start = .; *(.init0); __init_0_end = .;
        __init_1_start = .; *(.init1); __init_1_end = .;
        __init_end = .;

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
