#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include "../pci/pci.h"

#define MLBAR 0 // PCI BAR0
#define MUBAR 1 // PCI BAR1

// CQ/SQ Constants
#define NVME_DEFAULT_AQUEUE_SIZE 4095   // Default Admin Queue Size
#define NVME_DEFAULT_IOQUEUE_SIZE 4095  // Default IO Queue Size (forSQ)
#define NVME_MAX_SUPPORTED_IOQUEUES 256
#define NVME_CC_IOCQES 4 // Value of NVME_CC_IOCQES
#define NVME_CC_IOSQES 6 // Value of NVME_CC_IOSQES
#define NVME_CC_COMMAND_SET 0x0 // NVM Command Set

// MLBAR offsets
#define NVME_CAP 0x0        // Controller Capabilities
#define NVME_INTMS 0xC      // Interrupt Mask Set
#define NVME_INTMC 0xF      // Interrupt Mask Clear
#define NVME_CC 0x14        // Controller Configuration
#define NVME_AQA 0x24       // Admin Queue Attributes
#define NVME_ASQ 0x28       // Admin Submission Queue Base Address
#define NVME_ACQ 0x30       // Admin Completion Queue Base Address

static struct NVME_Drive nvme;

struct NVME_IOQueue {
    uint64_t base;
    uint64_t len;
};

struct NVME_Drive {
    // General PCI information
    struct PCI_Device* pci;

    // Admin Controller Information
    uint64_t acq_base_addr; // Admin Completion Queue Base Address
    uint64_t asq_base_addr; // Admin Submission Queue Base Address
    uint64_t admin_queue_len;

    // IO Controller Information
    // With MMIO mode, NVMe can have n:1 (iosq:iocq) ratio
    struct NVME_IOQueue iocq;                                   // The IO CQ
    struct NVME_IOQueue iosqs[NVME_MAX_SUPPORTED_IOQUEUES];     // The IO SQ's
    uint16_t cur_ioqueues;  // How many IO Submission Queues we have
};

void nvme_init();
void nvme_setup_queues(struct NVME_Drive* nvme_dev);   // Initialize IO SQ's, IO CQ's, etc, etc

#endif // NVME_H