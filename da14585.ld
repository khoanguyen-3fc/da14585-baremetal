/* Link map for a DA14585 image (0x07FC0000, 96 KB).
 *   0x07FC0000  .isr_vector + code + rodata   (this image, a few hundred bytes)
 *   ..0x07FD8000  stack, growing down from the top of RAM
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
    } > RAM

    /* startup.s always runs its single-section .data copy loop, so it needs
     * these three symbols even though .data is empty (it copies 0 bytes). */
    __etext = ALIGN(4);
    .data : { __data_start__ = .; *(.data*); __data_end__ = .; } > RAM
    ASSERT(SIZEOF(.data) == 0, "blinky must have no initialised .data")

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    } > RAM

    /* No exception unwinder in this -nostdlib C++ build. */
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) }

    ASSERT(. <= __StackTop, "region RAM overflowed with stack")
}
