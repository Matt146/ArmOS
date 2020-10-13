asm("jmp kmain\n\t");

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


