/* Link map for a DA14585 image (0x07FC0000, 96 KB).
 *   0x07FC0000  .isr_vector + code + rodata + .init_array + .data
 *               .bss  (NOLOAD -- no image bytes, zeroed by startup.s)
 *   ..0x07FD8000  stack, growing down from the top of RAM
 *
 * RAM-only image: .data's load address == its run address.
 * .bss clearing needs -D__STARTUP_CLEAR_BSS (see Makefile).
 * startup.s copies and zeroes 32-bit words, so .data and .bss are word aligned
 * and word sized.
 */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x07FC0000, LENGTH = 96K
}

__StackTop = ORIGIN(RAM) + LENGTH(RAM);   /* isr_vector[0]; stack grows down */

ENTRY(Reset_Handler)

SECTIONS
{
    .text 0x07FC0000 :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata .rodata.*)
        . = ALIGN(4);
    } > RAM

    /* C++ global constructors, collected so they aren't an orphan section.
     * NOTHING RUNS THEM (stock startup.s, no C library _start), so a global
     * object with a constructor stays unconstructed -- plain globals are fine.
     * To support them, walk this table in _start() before main(). */
    .init_array :
    {
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    } > RAM

    .data :
    {
        . = ALIGN(4);
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM

    __etext = __data_start__;   /* startup.s copy source == destination */

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    /* No exception unwinder in this -nostdlib C++ build. */
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) }

    ASSERT(. <= __StackTop, "region RAM overflowed with stack")
}
