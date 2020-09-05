ENTRY(exec_start)

OUTPUT_FORMAT(elf64-x86-64)

KRNL_VMA_START = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KRNL_VMA_START + 0x100000;

    .stivalehdr : ALIGN(4K) {
        KEEP(*(.stivalehdr))
    }

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(.bss*)
        *(COMMON)
    }
}
