
MEMORY
{
    rom (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    ccm (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

ENTRY(_start)

PROVIDE(__rom_start	= ORIGIN(rom));
PROVIDE(__rom_size	= LENGTH(rom));
PROVIDE(__rom_end	= ORIGIN(rom) + LENGTH(rom));

PROVIDE(__ram_start	= ORIGIN(ram));
PROVIDE(__ram_size	= LENGTH(ram));
PROVIDE(__ram_end       = ORIGIN(ram) + __ram_size);

PROVIDE(__stack_start	= ORIGIN(ccm));
PROVIDE(__stack_size	= LENGTH(ccm));
PROVIDE(__stack_end	= ORIGIN(ccm) + LENGTH(ccm));

PROVIDE(__heap_start	= __bss_end);
PROVIDE(__heap_end	= __ram_end);
PROVIDE(__heap_size     = __ram_end - __bss_end);

SECTIONS
{
    /* Make sure our ISR is at the very beginning */
    .isr_vector_table :
    {
        __isr_vector_table_start = .;
        KEEP(*(.isr_vector_table))
        ASSERT (. != __isr_vector_table_start, "No ISR vector table found, please provide one in section .isr_vector_table!");
    } >rom

    .text :
    {
        *(.text .text.* .gnu.linkonce.t.*)
        *(.plt)
        *(.gnu.warning)
        *(.glue_7t) *(.glue_7) *(.vfp11_veneer)

        *(.rodata .rodata.* .gnu.linkonce.r.*)

        *(.ARM.extab* .gnu.linkonce.armextab.*)
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)

        . = ALIGN(4);
        KEEP(*(.init))

        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        . = ALIGN(0x4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        . = ALIGN(4);
        KEEP(*(.fini))

        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;

        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))

    } >rom

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >rom
    __exidx_end = .;
    .text.align :
    {
        . = ALIGN(8);
        _etext = .;
    } >rom

    .data :
    {
        __data_start = .;
        *(.cs3.region-head.ram)
        KEEP(*(.jcr))
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        *(.ram)
        . = ALIGN(4);
        __data_end = .;
    } >ram AT>rom

    .bss :
    {
        __bss_start = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        *(.ram.b)
        . = ALIGN(4);
        __bss_end = .;
    } >ram

    .stack :
    {
        *(.stack)
    } >ccm

    __data_rom_start = LOADADDR (.data);

    .stab 0 (NOLOAD) : { *(.stab) }
    .stabstr 0 (NOLOAD) : { *(.stabstr) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }

    .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
    .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
    /DISCARD/ : { *(.note.GNU-stack) }
}
