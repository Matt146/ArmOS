#include "pci.h"

uint32_t pci_legacy_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset) {
    // 31: enable bit
    // 16..23: bus
    // 11..15: device
    // 8..10: function
    // 0..7: register offset
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    uint32_t address = (uint32_t)(1 << 31) | (lbus << 16) | (ldevice << 11) | (lfunction << 8) | reg_offset;
    out_d(PCI_CONFIG_ADDRESS, address);
    return in_d(PCI_CONFIG_DATA);
}

void pci_legacy_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset, uint32_t data) {
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    uint32_t address = (uint32_t)(1 << 31) | (lbus << 16) | (ldevice << 11) | (lfunction << 8) | reg_offset;
    out_d(PCI_CONFIG_ADDRESS, address);
    out_d(PCI_CONFIG_DATA, data);
}

static bool pci_device_exists(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t vendor_id = (uint32_t)(pci_legacy_read(bus, device, function, 0) & 0xffff);
    if (vendor_id == 0xffff) {
        return false;
    }
    return true;
}

static bool pci_device_is_multifunction(uint8_t bus, uint8_t device) {
    uint8_t header_type = pci_get_header_type(bus, device, 0);
    if ((header_type & 0x80) != 0) {
        return true;
    }
    return false;
}

static uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function) {
    return (uint8_t)((pci_legacy_read(bus, device, function, 0xC) >> 16) & 0xFF);
}

static uint64_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function) {
    pci_devices[cur_pci_device].device_id = (uint16_t)(pci_legacy_read(bus, device, function, 0) >> 16);
    pci_devices[cur_pci_device].vendor_id = (uint16_t)(pci_legacy_read(bus, device, function, 0) & 0xffff);
    pci_devices[cur_pci_device].bus = bus;
    pci_devices[cur_pci_device].device = device;
    pci_devices[cur_pci_device].function = function;
    for (size_t i = 0; i < 6; i++) {
        // Read the bar in question
        uint32_t bar = pci_legacy_read(bus, device, function, 0x10 + (i * 4));

        // Determine if it's mmio or port IO
        bool mmio = false;
        if ((bar & 0x1) == 0) {
            mmio = true;
        } else {
            mmio = false;
        }

        // Determine if the bar is 32-bit or 64-bit base address from the type field
        if ((bar & 0x6) == 0x0) {
            // 32-bit
            // get the bar base
            uint32_t bar_base = bar & 0xFFFFFFF0;

            // write a value of all 1's to the register temporarily
            pci_legacy_write(bus, device, function, 0x10 + (i * 4), 0xFFFFFFFF);
            uint32_t bar_limit = pci_legacy_read(bus, device, function, 0x10 + (i * 4)); // read the value back, mask the information bits, and perform a two's complement of that to get the limit
            bar_limit = (~(bar_limit & 0xFFFFFFF0)) + 1;    // this is the bar limit
            pci_legacy_write(bus, device, function, 0x10 + (i * 4), bar);
            pci_devices[cur_pci_device].bars[i].addr = (uint64_t)bar_base;
            pci_devices[cur_pci_device].bars[i].len = (uint64_t)bar_limit;
            pci_devices[cur_pci_device].bars[i].is_mmio = mmio;
            continue;
        } else if ((bar & 0x6) == 0x2) {
            // 64-bit means it consumes the BAR below it as well
            uint64_t bar_base = (bar & 0xFFFFFFF0) + ((pci_legacy_read(bus, device, function, 0x10 + (i * 4) + 1) & 0xFFFFFFFF) << 32);
            uint32_t bar_limit = pci_legacy_read(bus, device, function, 0x10 + (i * 4)); // read the value back, mask the information bits, and perform a two's complement of that to get the limit
            bar_limit = (~(bar_limit & 0xFFFFFFF0)) + 1;    // this is the bar limit
            pci_legacy_write(bus, device, function, 0x10 + (i * 4), bar);
            pci_devices[cur_pci_device].bars[i].addr = (uint64_t)bar_base;
            pci_devices[cur_pci_device].bars[i].len = (uint64_t)bar_limit;
            pci_devices[cur_pci_device].bars[i].is_mmio = mmio;
            i += 1;
            continue;
        }
    }

    cur_pci_device += 1;
    return 0;
}

void pci_scan_devices() {
    // bus
    for (size_t x = 0; x < 256; x++) {
        // device
        for (size_t y = 0; y < 32; y++) {
            if (pci_device_exists(x, y, 0) == true) {
                if (pci_device_is_multifunction(x, y) == true) {
                    for (size_t z = 0; z < 8; z++) {
                        if (pci_device_exists(x, y, z) == true) {
                            if ((pci_get_header_type(x, y, z) & 0x7F) == 0) {
                                pci_get_device(x, y, z);
                            }
                        }
                    }
                    continue;
                } else {
                    if ((pci_get_header_type(x, y, 0) & 0x7F) == 0) {
                        pci_get_device(x, y, 0);
                    }
                }
            }
        }
    }

    pci_debug_devices();
}

static void pci_debug_devices() {
    serial_puts("[PCI - DEBUG] DEVICES DETECTED TOTAL: ");
    serial_puts(unsigned_long_to_str(cur_pci_device));
    for (size_t i = 0; i < cur_pci_device; i++) {
        serial_puts("\n[PCI - DEBUG]: Device:");
        serial_puts("\n - Device ID: ");
        serial_puts(unsigned_long_to_str((uint64_t)pci_devices[i].device_id));
        serial_puts("\n - Vendor ID: ");
        serial_puts(unsigned_long_to_str((uint64_t)pci_devices[i].vendor_id));
        serial_puts("\n - Bus: ");
        serial_puts(unsigned_long_to_str((uint64_t)pci_devices[i].bus));
        serial_puts("\n - Device: ");
        serial_puts(unsigned_long_to_str((uint64_t)pci_devices[i].device));
        serial_puts("\n - Function: ");
        serial_puts(unsigned_long_to_str((uint64_t)pci_devices[i].function));
        serial_puts("\n - BAR's:");
        for (size_t j = 0; j < 6; j++) {
            serial_puts("\n\t - Bar #");
            serial_puts(unsigned_long_to_str(j));
            serial_puts(": ");
            serial_puts("\n\t - Base Address: ");
            serial_puts(unsigned_long_to_str(pci_devices[i].bars[j].addr));
            serial_puts("\n\t - Length: ");
            serial_puts(unsigned_long_to_str(pci_devices[i].bars[j].len));
            serial_puts("\n\t - Is MMIO: ");
            if (pci_devices[i].bars[j].is_mmio == true) {
                serial_puts("true");
            } else {
                serial_puts("false");
            }
        }
    }
}