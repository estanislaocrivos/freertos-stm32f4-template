/*
 * Linker script para STM32F401xE
 *
 * Memoria del STM32F401RE:
 * - Flash: 512KB @ 0x08000000
 * - SRAM:   96KB @ 0x20000000
 */

/* Punto de entrada - el símbolo donde empieza la ejecución */
ENTRY(Reset_Handler)

/* Stack inicial - al final de la RAM (crece hacia abajo) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Tamaño mínimo de heap y stack */
_Min_Heap_Size = 0x200;   /* 512 bytes - FreeRTOS usa su propio heap */
_Min_Stack_Size = 0x400;  /* 1KB para el stack inicial (antes del scheduler) */

/* Definición de regiones de memoria */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

/* Secciones - cómo organizar el código/datos en memoria */
SECTIONS
{
    /* Vector table - DEBE estar al inicio de Flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))  /* KEEP evita que el linker lo elimine */
        . = ALIGN(4);
    } >FLASH

    /* Código del programa */
    .text :
    {
        . = ALIGN(4);
        *(.text)           /* Código normal */
        *(.text*)          /* Código con sufijos (.text.startup, etc) */
        *(.glue_7)         /* Glue ARM/Thumb */
        *(.glue_7t)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        _etext = .;        /* Símbolo: fin del código */
    } >FLASH

    /* Datos de solo lectura (const, strings) */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* Constructores C++ */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } >FLASH

    /*
     * Datos inicializados (.data)
     * Valores almacenados en Flash, copiados a RAM en startup
     */
    _sidata = LOADADDR(.data);  /* Dirección en Flash donde están los valores */

    .data :
    {
        . = ALIGN(4);
        _sdata = .;        /* Inicio de .data en RAM */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;        /* Fin de .data en RAM */
    } >RAM AT> FLASH       /* Va en RAM, pero se carga desde Flash */

    /*
     * Datos no inicializados (.bss)
     * Se ponen a cero en startup
     */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* Inicio de .bss */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;         /* Fin de .bss */
        __bss_end__ = _ebss;
    } >RAM

    /* Espacio para heap y stack */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM

    /* Descartar secciones innecesarias */
    /DISCARD/ :
    {
        *(.ARM.attributes)
        *(.comment)
        *(.note*)
    }
}
