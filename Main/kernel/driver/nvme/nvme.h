#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include "../pci/pci.h"

#define MLBAR 0 // PCI BAR0
#define MUBAR 1 // PCI BAR1

// NVME Queue Constants
#define NVME_DEFAULT_AQUEUE_SIZE 4096   // Default Admin Queue Size
#define NVME_DEFAULT_IOQUEUE_SIZE 4096  // Default IO Queue Size (forSQ)
#define NVME_DEFAULT_IOSQ_COUNT 0
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

    // Cur CID
    uint16_t cur_cid;
};

struct NVME_Namespace {
    uint32_t namespace_id;
    uint64_t namespace_size;
    uint64_t namespace_capacity;
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
    uint16_t mqes;                                              // Maximum entries supported per queue (CAP.MQES)
    uint64_t io_cq_entry_size;                                  // Size of IO Completion Queue Etnries (based on CC.IOCQES)
    uint64_t io_sq_entry_size;                                  // Size of IO Submission QUeue ENtries (based on CC>IOSQES)
    bool queue_entries_contiguous;                              // baed on CAP.CQR; Are IO Queue entries required to be contiguous
    uint16_t cur_iocq_qid;                                      // Current IO Completion Queue Queue Identifier
    uint16_t cur_iosq_qid;                                      // Current IO Submission Queue Queue Identifier
    struct NVME_Namespace namespaces[1024];
};

struct NVME_Command {
    // DWORD 0
    uint8_t opcode; // Command opcode
    uint8_t zero0;   // 9:8 are the fused operation stuff and 15:14 indicate prp or sgl, but for our purposes, we set it to 0
    uint16_t cid;   // Command identifier - unique for each command

    // Rest of command - common part (up to DWORD 9)
    uint32_t nsid;  // No namespace, should be cleared to 0. Otherwise, set to INT32_MAX
    uint64_t zero1;  // Reserved
    uint64_t mptr;   // Metadata pointer
    uint64_t prp1;  // Data pointer (part 1)
    uint64_t prp2;  // Data pointer (part 2)

    // DWORD's 10, 11, 12, 13, 14, 15
    uint32_t specifics[6];
} __attribute__((packed));

struct NVME_CQ_Entry {
    // DWORD 0
    uint32_t command_specific;  // Command-specific DWORD

    // DWORD 1
    uint32_t zero0;             // Reserved

    // DWORD 2
    uint16_t sq_head_ptr;       // Submission Queue Head Pointer (at the time the completion queue was created)
    uint16_t sq_identifier;     // Submission Queue Identifier (which submission queue this is from)

    // DWORD 3
    uint16_t cid;               // Command ID
    uint16_t status;            // Status Field - Bit 0 of this is the "P" bit
};


void nvme_init();

// NVMe General Queue Functions
struct NVME_CQ_Entry* nvme_submit_command(struct NVME_Drive* nvme_dev, struct NVME_IOQueue* sq, struct NVME_IOQueue* cq, struct NVME_Command* command);


// Admin Queue Commands
void nvme_create_iocq(struct NVME_Drive* nvme_dev);
void nvme_create_iosq(struct NVME_Drive* nvme_dev);
void nvme_detect_namespaces(struct NVME_Drive* nvme_dev);       // Detects up to the first 1024 nameespaces with the "Identify" command

// IO Quueue Commands
void nvme_write(struct NVME_Drive* nvme_dev);

// Debug Functions
void nvme_debug_command(struct NVME_Command* cmd);

#endif // NVME_H