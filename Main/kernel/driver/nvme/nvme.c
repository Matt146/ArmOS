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
    serial_puts("\n[NVME] Writing to AQA...");
    volatile uint32_t* aqa_ptr = (volatile uint32_t*)((uint64_t)nvme_dev->pci->bars[0].addr + NVME_AQA);
    *aqa_ptr |= (uint32_t)NVME_DEFAULT_AQUEUE_SIZE;
    nvme_dev->admin_queue_len = NVME_DEFAULT_AQUEUE_SIZE;
    serial_puts("\n[NVME] Writing to ACQ...");
    volatile uint32_t* acq_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_ACQ);
    *acq_ptr = pmm_alloc(1);
    nvme_dev->acq_base_addr = *acq_ptr;
    serial_puts("\n[NVME] Writing to ASQ...");
    volatile uint32_t* asq_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_ASQ);
    *asq_ptr = pmm_alloc(1);
    nvme_dev->asq_base_addr = *asq_ptr;

    // Configure the size of the IO CQ's and IO SQ's
    volatile uint32_t* cc_ptr = (volatile uint32_t*)(nvme_dev->pci->bars[0].addr + NVME_CC);
    *cc_ptr |= (NVME_CC_IOCQES << 20) | (NVME_CC_IOSQES << 16) | (NVME_CC_COMMAND_SET << 4);

    // Submit a "Set Features" command with the Number of Queues attribute set to the number of IO SQ's and
    // IO CQ's
}