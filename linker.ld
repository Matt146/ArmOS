OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kmain)

SECTIONS {
        /*. = 0x8c00;*/
        .text 0x8c00 : {
                *(.text)
        }
}
