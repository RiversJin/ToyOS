OUTPUT_ARCH("aarch64")
ENTRY( _entry )
SECTIONS{
    . = 0xFFFF000000080000;
    .text : {
        *(.text._entry)
        *(.text .text.*)
    }
    PROVIDE(text_end = .);
    .rodata : {
        . = ALIGN(16);
        *(.srodata .srodata.*)
        . = ALIGN(16);
        *(.rodata .rodata.*)
    }

    .data : {
        . = ALIGN(16);
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
    }

    .bss : {
        . = ALIGN(16);
        *(.sbss .sbss.*)
        . = ALIGN(16);
        *(.bss .bss.*)
    }

    PROVIDE(kernel_end = .);
}