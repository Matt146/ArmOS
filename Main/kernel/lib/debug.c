#include "debug.h"
#include "util.h"

void print_format(void (*put)(char), const char *str, va_list args) {
    for(size_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%') {
            put(str[i]);
        } else {
            switch(str[++i]) {
                case 'b': {
                    size_t arg = va_arg(args, long);
                    char *string = int_base_to_string(arg, 2);
                    for(size_t i = 0; i < strlen(string); i++) {
                        put(string[i]);
                    }
                    break;
                }
                case 'd': {
                    size_t arg = va_arg(args, long);
                    char *string = int_base_to_string(arg, 10);
                    for(size_t i = 0; i < strlen(string); i++) {
                        put(string[i]);
                    }
                    break;
                }
                case 'x': {
                    size_t arg = va_arg(args, long);
                    char *string = int_base_to_string(arg, 16);
                    for(size_t i = 0; i < strlen(string); i++) {
                        put(string[i]);
                    }
                    break;
                }
                case 'c': {
                    char character = va_arg(args, int);
                    put(character);
                    break;
                }
                case 's': {
                    const char *string = va_arg(args, const char*);
                    for(size_t i = 0; i < strlen(string); i++) {
                        put(string[i]);
                    }
                    break;
                }
            }
        }
    }
    va_end(args);
}

void print(const char *str, ...) {
    va_list args;
    va_start(args, str);

    print_format(serial_putc, str, args);
}
