int kmain() {
        volatile char* video = (volatile char*)0xb8000;
        *video++ = 'A';
        *video++ = 4;
        return 0xbaba;
}
