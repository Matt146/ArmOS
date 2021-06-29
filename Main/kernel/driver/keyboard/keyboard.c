#include "keyboard.h"

void ps2_write(uint16_t data) {
    // Poll for input buffer status bit to be clear
    while (in_b(PS2_STATUS) & 0x2 != 0) {

    }

    serial_puts("\n[PS2] Writing to ps2 controller: ");
    serial_puts(unsigned_long_to_str(data));

    // Write the first 8 bits to 0x64 - our command register
    serial_puts("\n - First byte: ");
    serial_puts(unsigned_long_to_str((uint64_t)(data & 0xFF)));
    out_b(PS2_COMMAND, (uint8_t)(data & 0xFF));

    // If we need to, write the second byte to the next byte
    if ((data & 0xFF00) != 0) {
        serial_puts("\n - Second byte: ");
        serial_puts(unsigned_long_to_str((uint64_t)(data >> 8)));
        out_b(PS2_DATA, (uint8_t)(data >> 8));
    }
}

uint8_t ps2_read() {
    // Poll for the output buffer bit to be set
    serial_puts("\n[PS2] READING PS2 INPUT.");
    while (in_b(PS2_STATUS) & 0x1 != 1) {

    }
    return in_b(PS2_DATA);
}

void ps2_keyboard_init() {
    // Disable the ps2 controller
    //serial_puts("\n[PS2] Disabling ps2 devices");
    ps2_write(0xAD);
    ps2_write(0xA7);

    serial_puts("\n[PS2] Flushing output buffer");
    ps2_read(); // Flush the output buffer

    // Enable the ps2 devices
    serial_puts("\n[PS2] Enabling ps2 devices");
    ps2_write(0xAE);  // Enable 1st ps2 device
    ps2_write(0xA8);  // Enable 2nd ps2 device

    // Set the controller configuration byte to support interrupts
    serial_puts("\n[PS2] Setting controller configuration byte to support interrupts");
    ps2_write(0x60 | (0x3 << 8));
    serial_puts("\n[PS2] Absolute bruh moment.");
}

void ps2_irq1_handler(void) {
    uint8_t data = ps2_read();
    serial_puts("\n[KEYBOARD - PS2] INCOMING SCANCODE: ");
    serial_puts(unsigned_long_to_str((uint64_t)data));
    lapic_signal_eoi();
}