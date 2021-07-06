#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include "../pci/pci.h"

#define MLBAR 0 // PCI BAR0
#define MUBAR 1 // PCI BAR1

// NVME Queue Constants
#define NVME_DEFAULT_AQUEUE_SIZE 4095   // Default Admin Queue Size
#define NVME_DEFAULT_IOQUEUE_SIZE 4095  // Default IO Queue Size (forSQ)
#define NVME_DEFAULT_IOSQ_COUNT 5
#define NVME_MAX_SUPPORTED_IOQUEUES 256

// NVME Controller COnfiguration Constants
#define NVME_CC_IOCQES 4 // Value of NVME_CC_IOCQES (16-byte entries)
#define NVME_CC_IOSQES 6 // Value of NVME_CC_IOSQES (64-byte entries)
#define NVME_CC_AMS 0x0  // Value of the CC Arbitration Mechanism Selected
#define NVME_CC_MPS 0x0  // Value of the CC Memory Page Size (default page size on our arch, in our case 4K) (Calculated by 2^(12 + NVME_CC_MPS))
#define NVME_CC_CSS 0x0  // NVM Command Set

// MLBAR offsets
#define NVME_CAP 0x0        // Controller Capabilities
#define NVME_INTMS 0xC      // Interrupt Mask Set
#define NVME_INTMC 0xF      // Interrupt Mask Clear
#define NVME_CC 0x14        // Controller Configuration
#define NVME_CSTS 0x1C      // Controller Status
#define NVME_AQA 0x24       // Admin Queue Attributes
#define NVME_ASQ 0x28       // Admin Submission Queue Base Address
#define NVME_ACQ 0x30       // Admin Completion Queue Base Address

static struct NVME_Drive nvme;

struct NVME_IOQueue {
    // Basic Queue Stuff
    uint64_t base;
    uint64_t len;

    // Doorbell stuff
    uint64_t doorbell_prev_value;
    uint64_t doorbell_register_addr;
};

struct NVME_Drive {
    // General PCI information
    struct PCI_Device* pci;

    // Admin Controller Information
    struct NVME_IOQueue asq_queue;
    struct NVME_IOQueue acq_queue;

    // IO Controller Information
    // With MMIO mode, NVMe can have n:1 (iosq:iocq) ratio
    struct NVME_IOQueue iocq;                                   // The IO CQ
    struct NVME_IOQueue iosqs[NVME_MAX_SUPPORTED_IOQUEUES];     // The IO SQ's
    uint16_t cur_ioqueues;                                      // How many IO Submission Queues we have
    uint8_t dstrd;                                              // Value of CAP.DSTRD (doorbell stride)
    uint16_t mqes;                                              // Maximum queue entry size (CAP.MQES)
    uint64_t io_cq_entry_size;                                  // Size of IO Completion Queue Etnries (based on CC.IOCQES)
    uint64_t io_sq_entry_size;                                  // Size of IO Submission QUeue ENtries (based on CC>IOSQES)
};

struct NVME_Command {
    // DWORD 0
    uint8_t opcode; // Command opcode
    uint8_t zero0;   // 9:8 are the fused operation stuff and 15:14 indicate prp or sgl, but for our purposes, we set it to 0
    uint16_t cid;   // Command identifier - unique for each command

    // Rest of command - common part (up to DWORD 9)
    uint32_t nsid;  // No namespace, should be cleared to 0. Otherwise, set to INT32_MAX
    uint64_t zero1;  // Reserved
    uint8_t mptr;   // Metadata pointer
    uint64_t prp1;  // Data pointer (part 1)
    uint64_t prp2;  // Data pointer (part 2)

    // DWORD's 10, 11, 12, 13, 14, 15
    uint64_t specifics[6];
} __attribute__((packed));

void nvme_init();
void nvme_setup_queues(struct NVME_Drive* nvme_dev);   // Initialize IO SQ's, IO CQ's, etc, etc
void nvme_submit_command(struct NVME_IOQueue* sq, struct NVME_Command* command);

// Debug Functions
void nvme_debug_command(struct NVME_Command* cmd);

#endif // NVME_H