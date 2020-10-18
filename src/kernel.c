asm("jmp kmain\n\t");

// Unsigned integer typedefs
typedef         unsigned char                   uint8_t;
typedef         unsigned short int              uint16_t;
typedef         unsigned int                    uint32_t;
typedef         unsigned long long int          uint64_t;

// Signed integer typedefs
typedef         char            int8_t;
typedef         short int       int16_t;
typedef         int             int32_t;
typedef         long long int   int64_t;

struct IDTR {
        uint16_t limit; 
        uint64_t base;
} __attribute__((packed));

struct IDTGate {
        uint16_t        offset_1;       // offset bits 0..15
        uint16_t        selector;       // the code segment selector in GDT or LDT
        uint8_t         ist;            // bits 0..2 holds interrupt stack table offset, rest of bits 0
        uint8_t         type_attr;      // type and attributes
        uint16_t        offset_2;       // offset bits 16..31
        uint32_t        offset_3;       // offset bits 32..63
        uint32_t        zero;           // reserved
} __attribute__((packed));

int test_func_not_kmain() {
        volatile char* video = (volatile char*)0xb800A;
        *video++ = 'A';
        *video++ = 4;
        return 0xcaca;
}

int kmain() {
        volatile char* video = (volatile char*)0xb8000;
        *video++ = 'A';
        *video++ = 4;
        return 0xbaba;
}


