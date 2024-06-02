/* Entry Point */
ENTRY(Reset_Handler)

/* Heap and stack sizes in bytes for the application */
ld__heap_size  = 0;
ld__stack_size = 1k;

/* Memories definition */
MEMORY
{
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
    FLASH ( rx) : ORIGIN = 0x08000000,  LENGTH = 64K
}

/* Highest address of the user mode stack */
ld__estack = ORIGIN(RAM) + LENGTH(RAM); /* end of "RAM" Ram type memory */

SECTIONS
{
    /* ISR vector table section placed at address 0x00000000 in flash memory. */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* ISR Vector table */
        . = ALIGN(4);
    } >FLASH

    /* The program code and other data into "FLASH" Rom type memory */
    .text :
    {
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)

        /* Since these sections are not directly called by written code, the linker
        may think they are unreferenced and discard them. They are marked as KEEP
        to prevent this optimization. */
        KEEP (*(.init))   /* global/static constructors */
        
        /* On baremetal systems, there is usually not a return from main where the
        global/static destructors are called. To save flash, these functions are
        omitted from the linker script */
        /* KEEP (*(.fini)) */ /* global/static destructors */

        . = ALIGN(4);
        _etext = .;        /* define a global symbols at end of code */
    } >FLASH

    /* Constant data into "FLASH" Rom type memory */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >FLASH

    /* Exception unwinding information for stack unwinding during exepction
        handling in "FLASH" Rom type memory. */
    .ARM.extab   : {
        . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } >FLASH

    /* Index entry table used for stack unwinding during exception handling
        in "FLASH" Rom type memory. */
    .ARM : {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } >FLASH

    /* Initialization code that runs before global/static C++ constructors in
        "FLASH" Rom type memory. */
    .preinit_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
    } >FLASH

    /* Global/static C++ constructors in "FLASH" Rom type memory. */
    .init_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
    } >FLASH

    /* On baremetal systems, there is usually not a return from main where the
        global/static destructors are called. To save flash, these functions are
        omitted from the linker script */
    /* .fini_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
    } >FLASH */

    /* Used by the startup to initialize data */
    ld__sidata = LOADADDR(.data);

    /* Initialized data sections into "RAM" Ram type memory */
    .data :
    {
        . = ALIGN(4);
        ld__sdata = .;      /* create a global symbol at data start */
        *(.data)            /* .data sections */
        *(.data*)           /* .data* sections */
        *(.RamFunc)         /* .RamFunc sections */
        *(.RamFunc*)        /* .RamFunc* sections */

        . = ALIGN(4);
        ld__edata = .;       /* define a global symbol at data end */

    } >RAM AT> FLASH

    /* Uninitialized data section into "RAM" Ram type memory */
    . = ALIGN(4);
    .bss :
    {
        /* This is used by the startup in order to initialize the .bss section */
        ld__sbss = .;         /* define a global symbol at bss start */
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        ld__ebss = .;         /* define a global symbol at bss end */
    } >RAM

    /* ld__heap_stack_section section is used to check that there is enough "RAM"
        Ram type memory for the stack and heap. */
    .ld__heap_stack_section :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + ld__heap_size;
        . = . + ld__stack_size;
        . = ALIGN(8);
    } >RAM

    /* Remove information from the compiler libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
