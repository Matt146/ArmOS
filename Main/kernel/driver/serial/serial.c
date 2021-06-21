#include "serial.h"

void serial_init() {
    // 1. supply the baud rate
    // - The DLAB bit is the most significant bit of the "line control register," which
    // is +3 of the IO port offset. Setting the DLAB bit maps ports +0 and +1 to be the least
    // and most significant bytes of the divisor (respectively). If DLAB is cleared, ports +0 and +1
    // refer to the data register and the interrupt enable register.
    // - UART serial controller has an internal clock that runs at 115200 ticks per second
    // Clock divisor is used to control the baud rate
    // - To set the speed of the port, calculate the divisor required for the given baud rate and program
    // that into the divisor register.
    // - IE: Divisor of 1 will give you 115200 baud, 2 will give you 57600 baud, and 3 will give you 38400 baud
    out_b(COM1 + 0x3, 0x80);    // set DLAB
    out_b(COM1 + 0x0, DIVISOR_LOW); // set the low byte of the divisor now that the DLAB bit is set
    out_b(COM1 + 0x1, DIVISOR_HIGH); // set the high bit of the divisor now that the DLAB bit is set
    
    // 2. setup the parity type and the number of bits in a character
    // - 99% of the time you want 8 bits per character, no parity, and one stop bit and this
    // is what we'll be doing
    // - DATA BITS: the two least significant bits of the line control register are used to set the character length in bits
    // - ie: 00 = 5, 01=6, 10=7, 11=8
    // - STOP BTIS: to set the number of stop bits to 1, set bit 2 of the line control register to 0
    // - PARITY BTIS: used to see if interference happens and stuff (idc about this, so no parity bits)
    // - parity bits are controlled by bits 3, 4, and 5 of the line control register
    // - 000=none, 001=odd, 011=even, 101=mark, 111=space
    out_b(COM1 + 0x3, 0x03);    // 8 bits per character, 1 stop bit, no parity bits. Also clears DLAB
    

    // 3. disable interrupts for now, as we will be polling instead
    // havne't setup APIC yet that's why
    // COM1 and COM3 use IRQ #4
    // COM2 and COM4 use IRQ #3
    // write 0 to the interrupt-enable-register to make it so that interrupts are disabled
    out_b(COM1 + 0x2, 0x00); // disable all interrupts
}

bool serial_transmission_is_ready() {
    // check the line status register, specifically bit 5 (if set, the transmission buffer is empty,
    // meaning that data can be sent)
    return in_b(COM1 + 0x5) & 0x20;
}

void serial_putc(char c) {
    // busy loop while we wait for it to be ready
    while (serial_transmission_is_ready() != true) {

    }
    // now output the character
    out_b(COM1 + 0x0, c);
}

static uint8_t serial_mux;
void serial_puts(char* str) {
    mutex_lock(&serial_mux);
    size_t count = 0;
    while (true) {
        if (str[count] == '\0') {
            break;
        }
        serial_putc(str[count]);
        count += 1;
    }
    mutex_unlock(&serial_mux);
}
