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
    // Disable the controller
    volatile uint32_t* cc_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CC);
    *cc_ptr &= (~0x1);

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
    nvme_dev->asq_queue.iocqid = 0;
    nvme_dev->asq_queue.iosqid = 0;

    // Store NVME Admin Queue Doorbell/Queue Info
    volatile uint64_t* cap_ptr = (volatile uint64_t*)((uint64_t)nvme_dev->pci->bars[0].addr + NVME_CAP);
    nvme_dev->asq_queue.doorbell_register_addr = nvme_dev->pci->bars[0].addr + 0x1000;
    nvme_dev->dstrd = ((*cap_ptr >> 32) & 0b1111);
    nvme_dev->acq_queue.doorbell_register_addr = nvme_dev->pci->bars[0].addr + (0x1000 + (1 * 4 << nvme_dev->dstrd));

    // Configure the CC (controller configuration register) with CC.AMS (Arbitration Mechanism), CC.MPS (Memory Page Size)
    // CC.CSS( Command Set) before we set CC.EN. We also set CC.IOCQES/CC.IOSQES while we're here because they're required
    // to create IO Queues
    *cc_ptr |= (NVME_CC_IOCQES << 20) | (NVME_CC_IOSQES << 16) | (NVME_CC_AMS << 11) | (NVME_CC_MPS << 7) | (NVME_CC_CSS << 4);
    *cc_ptr |= 0x1;

    // Mask all controller IRQ's
    volatile uint32_t* intms_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_INTMS);
    *intms_ptr = 0xffffffff;

    // Determine the maximum entries supported per queue
    nvme_dev->mqes = (uint16_t)((*cap_ptr) & 0xffff);
    nvme_dev->mqes -= 1;
    serial_puts("\n[NVME] Maximum number of entries per queue: ");
    serial_puts(unsigned_long_to_str(nvme_dev->mqes));

    // Check the controller status to see what we did wrong (if we did)
    volatile uint32_t* csts_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CSTS);
    while ((*csts_ptr & (1 << 0)) != 1) {
        if ((*csts_ptr & (1<< 1)) == 1) {
            serial_puts("\n[NVME] NVME Controller fatal error... :(");
        }
    }
    serial_puts("\n[NVME] Enabled NVME controller and set CC Register");

    // Poll NVMe namespaces
    nvme_detect_namespaces(nvme_dev);

    // Submit "Set Features" command to set IO Queues size and stuff
    serial_puts("\n\n[NVME] Sending 'set features' command.");
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
    cmd.specifics[1] = (NVME_DEFAULT_IOSQ_COUNT << 16) | 0x0;
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;
    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->asq_queue), &(nvme_dev->acq_queue), &cmd), 1);

    // Create IO Queues
    serial_puts("\n\n[NVME] Creating IO Completion Queues!");
    nvme_create_iocq(nvme_dev);
    serial_puts("\n\n[NVME] Creating IO Submission Queues!");
    nvme_create_iosq(nvme_dev);

    // TEST NVME WRITE:
    nvme_write_block(nvme_dev, 10, nvme_dev->namespaces[0].namespace_id, "SCOOBY DOOBY DOO WHERE THE FUCK ARE YOU?", 40);

    // TEST NVME READ:
    uint64_t data_buff_ptr = nvme_read_block(nvme_dev, 10, nvme_dev->namespaces[0].namespace_id);
    serial_puts("\n\n\n[NVME] READ FROM DISK: ");
    serial_puts((char*)data_buff_ptr);
}

void nvme_debug_command(struct NVME_Command* cmd) {
    serial_puts("\n - Command: ");
    serial_puts("\n\t - sizeof(struct NVME_Command): ");
    serial_puts(unsigned_long_to_str(sizeof(struct NVME_Command)));
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

struct NVME_CQ_Entry* nvme_submit_command(struct NVME_Drive* nvme_dev, struct NVME_IOQueue* sq, struct NVME_IOQueue* cq, struct NVME_Command* command) {
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
    volatile struct NVME_Command* sq_ptr = (volatile struct NVME_Command*)(sq->base + (sq->doorbell_prev_value * pow(2, NVME_CC_IOSQES)));
    if (sq->doorbell_prev_value >= sq->len) {
        sq->doorbell_prev_value = 0;
        sq->cur_cid = 0;    // @FIXME @BUG HERE MAYBE????????
    }
    command->cid = sq->cur_cid;

    serial_puts("\n - Submission Queue Pointer Value: ");
    serial_puts(unsigned_long_to_str((uint64_t)sq_ptr));

    memcpy(sq_ptr, command, sizeof(struct NVME_Command));
    //nvme_debug_command((struct NVME_Command*)sq_ptr);
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

    sq->cur_cid += 1;

    // Wait for command to send
    while (true) {
        volatile uint32_t* csts_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CSTS);
        if (((*csts_ptr) & 0x1) == 1) {
            break;
        }
    }
    serial_puts("\n[NVME] Reading CQ Value...");
    nvme_debug_command(command);

    // NOW, get the corresponding CQ entry
    struct NVME_CQ_Entry* entry_ptr = (struct NVME_CQ_Entry*)(cq->base);
    bool found = false;
    while (true) {
        for (size_t i = 0; i < cq->len; i++) {
            if (entry_ptr->cid == sq->cur_cid - 1) {
                found = true;
                break;
            }
            entry_ptr += sizeof(struct NVME_CQ_Entry);  // @FIXME @BUG HERE??? SHOULD ADD BY THE QUEUE ENTRY SIZE, BUT WORKS FOR NOW, BUT HELLA DIRTY ASF
        }
        if (found == true) {
            break;
        }
    }
    struct NVME_CQ_Entry* entry_copied_ptr = (struct NVME_CQ_Entry*)pmm_alloc(1);
    memsetb(entry_copied_ptr, 0x0, PMM_PAGE_SIZE);
    memcpy(entry_copied_ptr, entry_ptr, sizeof(struct NVME_CQ_Entry));
    serial_puts("\n[NVME] Memory copied from CQ");
    volatile uint32_t* cq_doorbell_ptr = (volatile uint32_t*)(cq->doorbell_register_addr);
    *cq_doorbell_ptr = cq->doorbell_prev_value + 1;
    cq->doorbell_prev_value += 1;
    serial_puts("\n[NVME] Wrote to CQ Doorbell Register... Returning...");
    return entry_copied_ptr;
    return NULL;
}

void nvme_create_iocq(struct NVME_Drive* nvme_dev) {
    uint64_t cq_prp_ptr = pmm_alloc((nvme_dev->mqes * pow(2, NVME_CC_IOCQES)) % PMM_PAGE_SIZE);
    memsetb(cq_prp_ptr, 0x0, (nvme_dev->mqes * pow(2, NVME_CC_IOCQES)) * PMM_PAGE_SIZE);
    nvme_dev->cur_iocq_qid += 1;
    struct NVME_Command cmd;
    cmd.opcode = 0x5;       // "Create IOCQ" command opcode is 0x5
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = 0;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = cq_prp_ptr;
    cmd.prp2 = 0;
    cmd.specifics[0] = (nvme_dev->mqes << 16) | (nvme_dev->cur_iocq_qid); // Number of Queues feature ID
    cmd.specifics[1] = 0x0 | (1 << 0);     // BUG: YOU MAY WANT TO ENABLE INTERRUPTS HERE
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->asq_queue), &(nvme_dev->acq_queue), &cmd), 1);
    nvme_dev->iocq.base = (uint64_t*)cq_prp_ptr;
    nvme_dev->iocq.len = nvme_dev->mqes;
    nvme_dev->iocq.doorbell_prev_value = 0;
    //nvme_dev->iocq.doorbell_register_addr = nvme_dev->pci->bars[0].addr + (3 * 4 << nvme_dev->dstrd);
    nvme_dev->iocq.iocqid = nvme_dev->cur_iocq_qid;
    nvme_dev->iocq.iosqid = 0;
    nvme_dev->iocq.doorbell_register_addr = 0x1000 + nvme_dev->pci->bars[0].addr + ((2 * nvme_dev->iocq.iocqid + 1) * (4 << nvme_dev->dstrd));
    nvme_dev->iocq.cur_cid = 0;
}

void nvme_create_iosq(struct NVME_Drive* nvme_dev) {
    uint64_t sq_prp_ptr = pmm_alloc((nvme_dev->mqes * pow(2, NVME_CC_IOSQES)) % PMM_PAGE_SIZE);
    memsetb(sq_prp_ptr, 0x0, (nvme_dev->mqes * pow(2, NVME_CC_IOSQES)) * PMM_PAGE_SIZE);
    nvme_dev->cur_iosq_qid += 1;
    struct NVME_Command cmd;
    cmd.opcode = 0x1;       // "Create IOSQ" command opcode is 0x1
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = 0;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = sq_prp_ptr;
    cmd.prp2 = 0;
    cmd.specifics[0] = (nvme_dev->mqes << 16) | (nvme_dev->cur_iosq_qid); // Number of Queues feature ID
    cmd.specifics[1] = 0x0 | (1 << 0) | (nvme_dev->cur_iocq_qid << 16);     // BUG: YOU MAY WANT TO ENABLE INTERRUPTS HERE
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->asq_queue), &(nvme_dev->acq_queue), &cmd), 1);
    nvme_dev->iosqs[0].base = (uint64_t*)sq_prp_ptr;
    nvme_dev->iosqs[0].len = nvme_dev->mqes;
    nvme_dev->iosqs[0].doorbell_prev_value = 0;
    //nvme_dev->iosqs[0].doorbell_register_addr = nvme_dev->pci->bars[0].addr + (3 * 4 << nvme_dev->dstrd);
    nvme_dev->iosqs[0].cur_cid = 0;
    nvme_dev->iosqs[0].iocqid = nvme_dev->cur_iocq_qid;
    nvme_dev->iosqs[0].iosqid = nvme_dev->cur_iosq_qid;
    nvme_dev->iosqs[0].doorbell_register_addr = 0x1000 + nvme_dev->pci->bars[0].addr + ((2 * nvme_dev->iosqs[0].iosqid) * (4 << nvme_dev->dstrd));
}

void nvme_detect_namespaces(struct NVME_Drive* nvme_dev) {
    serial_puts("\n[NVME] Detecting Namespaces");
    uint64_t identify_buffer = pmm_alloc(1);
    memsetb((uint8_t*)identify_buffer, 0x0, PMM_PAGE_SIZE);
    struct NVME_Command cmd;
    cmd.opcode = 0x6;       // "Identify command opcode is 0x6
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = 0;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = identify_buffer;
    cmd.prp2 = 0;
    cmd.specifics[0] = 2; // CNS - Controller or Namespace Structure - this one only for detecting namespaces - NOT SUPPORTED ON ALL HW - BE CAREFUL!!!!!!!!!!!!!!!!! - WILL NOT WORK ON VMWARE
    cmd.specifics[1] = 0;
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->asq_queue), &(nvme_dev->acq_queue), &cmd), 1);

    // Now parse the Identify buffer
    // We can only get a maximum of 1024 entries
    uint32_t* namespace_ptr_list = (uint32_t*)identify_buffer;
    for (size_t i = 0; i < 1024; i++) {
        nvme_dev->namespaces[i].namespace_id = namespace_ptr_list[i];
    }

    // Now, for each detected namespace, we're going to issue an IDENTIFY with CNS=0
    // to get the "Identify Namespace" data structure for the LBA ranges of the namespace
    memsetb((uint8_t*)identify_buffer, 0x0, PMM_PAGE_SIZE); // First, clear the identify_buffer
    cmd.opcode = 0x6;       // "Identify command opcode is 0x6
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = 0;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = identify_buffer;
    cmd.prp2 = 0;
    cmd.specifics[0] = 0; // CNS - Controller or Namespace Structure - this one only for detecting namespaces - NOT SUPPORTED ON ALL HW - BE CAREFUL!!!!!!!!!!!!!!!!! - WILL NOT WORK ON VMWARE
    cmd.specifics[1] = 0;
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;
    uint64_t* identify_ptr = (uint64_t*)identify_buffer;
    for (size_t i = 0; i < 1024; i++) {
        if (nvme_dev->namespaces[i].namespace_id == 0) {
            break;
        }
        cmd.nsid = nvme_dev->namespaces[i].namespace_id;
        pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->asq_queue), &(nvme_dev->acq_queue), &cmd), 1);
        nvme_dev->namespaces[i].namespace_size = *identify_ptr;
        identify_ptr += sizeof(uint64_t);
        nvme_dev->namespaces[i].namespace_capacity = *identify_ptr;
    }

    // DEBUG LOG OF NAMESPACES
    for (size_t i = 0; i < 1024; i++) {
        if (nvme_dev->namespaces[i].namespace_id == 0) {
            break;
        }
        serial_puts("\n[NVME] Detected NSID: ");
        serial_puts(unsigned_long_to_str((uint64_t)nvme_dev->namespaces[i].namespace_id));
        serial_puts("\n - Namespace Size: ");
        serial_puts(unsigned_long_to_str((uint64_t)nvme_dev->namespaces[i].namespace_size));
        serial_puts("\n - Namespace Capacity: ");
        serial_puts(unsigned_long_to_str((uint64_t)nvme_dev->namespaces[i].namespace_capacity));
    }

    // Free the identify buffer
    pmm_free(identify_buffer, 1);
}

uint64_t nvme_read_block(struct NVME_Drive* nvme_dev, uint64_t lba, uint32_t nsid) {
    serial_puts("\n[NVME] READING BLOCK:");
    uint64_t buff = pmm_alloc(1);
    memsetb((uint8_t*)buff, 0x0, PMM_PAGE_SIZE);

    struct NVME_Command cmd;
    cmd.opcode = 0x2;       // 0x2 = command for read
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = nsid;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = buff;
    cmd.prp2 = 0;
    cmd.specifics[0] = (uint32_t)(lba & 0xffffffff);
    cmd.specifics[1] = (uint32_t)(lba >> 32);
    cmd.specifics[2] = 0x1;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    // Send the command now
    nvme_submit_command(nvme_dev, &(nvme_dev->iosqs[0]), &(nvme_dev->iocq), &cmd);
    serial_puts("\n[NVME] Read block...:");

    // Return buffer
    return buff;
}

void nvme_write_block(struct NVME_Drive* nvme_dev, uint64_t lba, uint32_t nsid, uint8_t* data, uint64_t data_len) {
    serial_puts("\n[NVME] WRITING BLOCK:");
    uint64_t buff = pmm_alloc(1);
    memsetb(buff, 0x0, PMM_PAGE_SIZE);
    memcpy(buff, data, data_len);

    struct NVME_Command cmd;
    cmd.opcode = 0x1;       // 0x1 = command for write
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = nsid;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = buff;
    cmd.prp2 = 0;
    cmd.specifics[0] = (uint32_t)(lba & 0xffffffff);
    cmd.specifics[1] = (uint32_t)(lba >> 32);
    cmd.specifics[2] = 0x1;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    // Send the command now
    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->iosqs[0]), &(nvme_dev->iocq), &cmd), 1);
    serial_puts("\n[NVME] WROTE block...:");
    serial_puts("\n[NVME] FLUSHING CHANGES:");

    cmd.opcode = 0x0;       // 0x0 = command for flush
    cmd.zero0 = 0;
    cmd.cid = 0;
    cmd.nsid = nsid;
    cmd.zero1 = 0;
    cmd.mptr = 0;
    cmd.prp1 = 0;
    cmd.prp2 = 0;
    cmd.specifics[0] = 0;
    cmd.specifics[1] = 0;
    cmd.specifics[2] = 0;
    cmd.specifics[3] = 0;
    cmd.specifics[4] = 0;
    cmd.specifics[5] = 0;

    pmm_free(nvme_submit_command(nvme_dev, &(nvme_dev->iosqs[0]), &(nvme_dev->iocq), &cmd), 1);
    serial_puts("\n[NVME] FLUSHED block...:");

    return buff;
}