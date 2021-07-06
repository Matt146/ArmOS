#include "nvme.h"

void nvme_init() {
    serial_puts("\n[NVME] Initializing NVME...");
    nvme.pci = pci_search_for_device(0x1, 0x8);     // NVMe devices have a PCI class code of 0x1 and a subclass code of 0x8
    if (nvme.pci != NULL) {
        serial_puts("\n[NVME] Found NVME Drive: ");
        pci_debug_device(nvme.pci);
        pci_become_busmaster(nvme.pci->bus, nvme.pci->device, nvme.pci->function);
        nvme_setup_queues(&nvme);
    } else {
        serial_puts("\n[NVME - error] No NVME Drives found :(");
    }
}

void nvme_setup_queues(struct NVME_Drive* nvme_dev) {
    // Configure Admin SQ and CQ by initializing the AQA, ASQ and ACQ
    // We do this before we set CC because you can't access these registers when you set CC.EN
    serial_puts("\n[NVME] Writing to AQA...");
    volatile uint32_t* aqa_ptr = (volatile uint32_t*)((uint64_t)nvme_dev->pci->bars[0].addr + NVME_AQA);
    *aqa_ptr |= NVME_DEFAULT_AQUEUE_SIZE | (NVME_DEFAULT_AQUEUE_SIZE << 16);
    nvme_dev->acq_queue.len = NVME_DEFAULT_AQUEUE_SIZE;
    nvme_dev->asq_queue.len = NVME_DEFAULT_AQUEUE_SIZE;
    serial_puts("\n[NVME] Writing to ACQ...");
    volatile uint32_t* acq_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_ACQ);
    *acq_ptr = pmm_alloc(1);
    nvme_dev->acq_queue.base = *acq_ptr;
    serial_puts("\n[NVME] Writing to ASQ...");
    volatile uint32_t* asq_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_ASQ);
    *asq_ptr = pmm_alloc(1);
    nvme_dev->asq_queue.base = *asq_ptr;

    // Store NVME Admin Queue Doorbell/Queue Info
    volatile uint64_t* cap_ptr = (volatile uint64_t*)((uint64_t)nvme_dev->pci->bars[0].addr + NVME_CAP);
    nvme_dev->asq_queue.doorbell_register_addr = nvme_dev->pci->bars[0].addr + 0x1000;
    nvme_dev->dstrd = ((*cap_ptr >> 32) & 0b1111);
    nvme_dev->acq_queue.doorbell_register_addr = nvme_dev->pci->bars[0].addr + (0x1000 + (1 * 4 << nvme_dev->dstrd));

    // Configure the CC (controller configuration register) with CC.AMS (Arbitration Mechanism), CC.MPS (Memory Page Size)
    // CC.CSS( Command Set) before we set CC.EN. We also set CC.IOCQES/CC.IOSQES while we're here because they're required
    // to create IO Queues
    volatile uint32_t* cc_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CC);
    *cc_ptr |= (NVME_CC_IOCQES << 20) | (NVME_CC_IOSQES << 16) | (NVME_CC_AMS << 11) | (NVME_CC_MPS << 7) | (NVME_CC_CSS << 4) | 0x1;

    // Check the controller status to see what we did wrong (if we did)
    volatile uint32_t* csts_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CSTS);
    while ((*csts_ptr & (1 << 0)) != 1) {
        if ((*csts_ptr & (1<< 1)) == 1) {
            serial_puts("\n[NVME] NVME Controller fatal error... :(");
        }
    }
    serial_puts("\n[NVME] Enabled NVME controller and set CC Register");

    // Poll NVMe namespaces

    // Submit "Set Features" command to set IO Queues size and stuff
    serial_puts("\n[NVME] Sending 'set features' command.");
    struct NVME_Command cmd;
    cmd.opcode = 0x9;       // "Set Features" command opcode is 0x9
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = 0;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = 0;
    cmd.prp2 = 0;
    cmd.specifics[0] = 0x7; // Number of Queues feature ID
    cmd.specifics[1] = (0x1 << 16) | NVME_DEFAULT_IOSQ_COUNT;
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;
    nvme_submit_command(&(nvme_dev->asq_queue), &cmd);

    // Create IO Queues
}

void nvme_debug_command(struct NVME_Command* cmd) {
    serial_puts("\n - Command: ");
    serial_puts("\n\t - Opcode: ");
    serial_puts(unsigned_long_to_str(cmd->opcode));
    serial_puts("\n\t - Zero0: ");
    serial_puts(unsigned_long_to_str(cmd->zero0));
    serial_puts("\n\t - CID: ");
    serial_puts(unsigned_long_to_str(cmd->cid));
    serial_puts("\n\t - NSID: ");
    serial_puts(unsigned_long_to_str(cmd->nsid));
    serial_puts("\n\t - Zero1: ");
    serial_puts(unsigned_long_to_str(cmd->zero1));
    serial_puts("\n\t - Mptr: ");
    serial_puts(unsigned_long_to_str(cmd->mptr));
    serial_puts("\n\t - PRP1: ");
    serial_puts(unsigned_long_to_str(cmd->prp1));
    serial_puts("\n\t - PRP2: ");
    serial_puts(unsigned_long_to_str(cmd->prp2));

    serial_puts("\n\t - Specifics (DWORD's 10-15): ");
    for (size_t i = 0; i < 6; i++) {
        serial_puts("\n\t\t - DWORD");
        serial_puts(unsigned_long_to_str(i));
        serial_puts(": ");
        serial_puts(unsigned_long_to_str(cmd->specifics[i]));
    }
}

void nvme_submit_command(struct NVME_IOQueue* sq, struct NVME_Command* command) {
    // Print Debug Information About the Queue
    serial_puts("\n - BEFORE: Queue Status: ");
    serial_puts("\n\t - Queue Base: ");
    serial_puts(unsigned_long_to_str(sq->base));
    serial_puts("\n\t - Queue Length: ");
    serial_puts(unsigned_long_to_str(sq->len));
    serial_puts("\n\t - Queue Doorbell Address: ");
    serial_puts(unsigned_long_to_str(sq->doorbell_register_addr));
    serial_puts("\n\t - Queue Doorbell Previous Value: ");
    serial_puts(unsigned_long_to_str(sq->doorbell_prev_value));

    // Print Debug information about the command
    nvme_debug_command(command);
    volatile struct NVME_Command* sq_ptr = (volatile struct NVME_Command*)(sq->base + (sq->doorbell_prev_value * NVME_CC_IOSQES));
    if (sq->doorbell_prev_value >= sq->len) {
        sq->doorbell_prev_value = 0;
    }

    memcpy(sq_ptr, command, sizeof(struct NVME_Command));
    volatile uint32_t* sq_doorbell_ptr = (volatile uint32_t*)(sq->doorbell_register_addr);
    *sq_doorbell_ptr = (sq->doorbell_prev_value + 1);
    sq->doorbell_prev_value += 1;

     // Print Debug Information About the Queue
    serial_puts("\n - AFTER: Queue Status: ");
    serial_puts("\n\t - Queue Base: ");
    serial_puts(unsigned_long_to_str(sq->base));
    serial_puts("\n\t - Queue Length: ");
    serial_puts(unsigned_long_to_str(sq->len));
    serial_puts("\n\t - Queue Doorbell Address: ");
    serial_puts(unsigned_long_to_str(sq->doorbell_register_addr));
    serial_puts("\n\t - Queue Doorbell Previous Value: ");
    serial_puts(unsigned_long_to_str(sq->doorbell_prev_value));
}