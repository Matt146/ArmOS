#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdbool.h>
#include "../../mm/pmm.h"
#include "../../lib/io.h"
#include "../../lib/util.h"
#include "../../int/lapic.h"

// CAM stuff = Configuration Address Space
// This is the legacy stuff
#define PCI_CONFIG_ADDRESS 0xCF8    // Specifies the configuration address that is required to be accesses
#define PCI_CONFIG_DATA 0xCFC       // Will actually generate the configuration access and will transfer the data to or from teh CONFIG_DATA register
#define PCI_LEGACY_CAM_SIZE 256
#define PCI_MAX_DEVICES 65536

#define PCI_LEGACY_HEADER_TYPE_OFFSET 0x10  // Always is 0x10

struct PCI_BAR {
    uint64_t addr;
    uint64_t len;
    bool is_mmio;
} __attribute__((packed));

struct PCI_Device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t device_id;
    uint16_t vendor_id;
    uint8_t _class;
    uint8_t subclass;

    struct PCI_BAR bars[6];
} __attribute__((packed));

static uint8_t pci_legacy_cam_buffer[256];
static struct PCI_Device pci_devices[PCI_MAX_DEVICES];
static uint64_t cur_pci_device;

// PCI Read/Write Basic Functions
uint32_t pci_legacy_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset);
void pci_legacy_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset, uint32_t data);

// PCI Enumeration Functions
static bool pci_device_exists(uint8_t bus, uint8_t device, uint8_t function);
static bool pci_device_is_multifunction(uint8_t bus, uint8_t device);
static uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function);
static uint64_t pci_get_device(uint8_t bus, uint8_t device, uint8_t function);
static void pci_debug_devices();
static void pci_init_msi(uint8_t bus, uint8_t device, uint8_t function, uint8_t irq);
void pci_init_device(uint8_t bus, uint8_t device, uint8_t function);
struct PCI_Device* pci_search_for_device(uint8_t _class, uint8_t subclass);
void pci_debug_device(struct PCI_Device* dev);

// PCI Functions to Export - Setting command register bits, initializing the device, becoming the busmaster
void pci_scan_devices();
void pci_set_command_reg(uint8_t bus, uint8_t device, uint8_t function, uint16_t comamnd_reg_value);
uint16_t pci_get_command_reg(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_get_status_reg(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_class(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function);
void pci_become_busmaster(uint8_t bus, uint8_t device, uint8_t function);

// So this is how PCI works:
// We put the address of the PCI register we want into 0xCF8 and then put the value we want to write (32-bit)
// to that 32-bit vaue into 0xCFC. Address works like so:
// bit 31 = enable bit (set this always)
// bits 16..23 - bus
// bits 11-15 - device
// bits 8..10 - function
// bits 0..7 - register offset

#endif // PCI_H