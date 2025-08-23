OUTPUT_FORMAT("elf32-tradbigmips")
OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS
{
    . = 0x00000000;  /* Start address */
    
    .text : {
        *(.text)
        *(.text.*)
    }
    
    /* Discard all other sections */
    /DISCARD/ : {
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
        *(.debug*)
        *(.gnu*)
        *(.hash*)
        *(.dynstr*)
        *(.dynsym*)
        *(.dynamic*)
        *(.interp*)
        *(.plt*)
        *(.got*)
        *(.data*)
        *(.bss*)
        *(.rodata*)
        *(.MIPS*)
        *(.reginfo*)
        *(.mdebug*)
    }
}
