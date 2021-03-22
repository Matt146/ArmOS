#include "e9.h"

void init_e9() {
	out_b(0xe9, (unsigned char)'A');
	unsigned char  e9_value = in_b(0xe9);
	if (e9_value == (unsigned char)'A') {
		e9_enabled = true;
	} else {
		e9_enabled = false;
	}
}

void e9_putc(char c) {
	out_b(0xe9, c);
}

void e9_puts(char* str) {
	size_t count = 0;
	while (true) {
		if (str[count] == '\0') {
			break;
		}
		e9_putc(str[count]);
        count += 1;
	}
}
