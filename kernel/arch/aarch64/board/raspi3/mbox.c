#include "mbox.h"
#include "../../arm.h"
#include "printf.h"

#define VIDEOCORE_MBOX  (MMIO_BASE + 0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x00))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

#define MBOX_TAG_GET_ARM_MEMORY     0x00010005
#define MBOX_TAG_GET_CLOCK_RATE     0x00030002
#define MBOX_TAG_END                0x0
#define MBOX_TAG_REQUEST

int mbox_read(uint8_t chan){
    while (1) {
        disb();
        while (*MBOX_STATUS & MBOX_EMPTY) ;
        disb();
        uint32_t r = *MBOX_READ;
        if ((r & 0xF) == chan) {
            cprintf("- mbox read: 0x%x\n", r);
            return r >> 4;
        }
    }
    disb();
    return 0;
}

void mbox_write(uint32_t buf, uint8_t chan){
    disb();
    assert((buf & 0xF) == 0 && (chan & ~0xF) == 0);
    while (*MBOX_STATUS & MBOX_FULL) ;
    disb();
    cprintf("- mbox write: 0x%x\n", (buf & ~0xF) | chan);
    *MBOX_WRITE = (buf & ~0xF) | chan;
    disb();
}

int mbox_get_arm_memory(){
    __attribute__((aligned(16)))
    uint32_t buf[] = {36, 0, MBOX_TAG_GET_ARM_MEMORY, 8, 0, 0, 0, MBOX_TAG_END};
    asserts((VA2PA(buf) & 0xF) == 0, "Buffer should align to 16 bytes. ");

    disb();
    dccivac(buf, sizeof(buf));
    disb();
    mbox_write(VA2PA(buf), 8);
    disb();
    mbox_read(8);
    disb();
    dccivac(buf, sizeof(buf));
    disb();

    asserts(buf[5] == 0, "Memory base address should be zero. ");
    asserts(buf[6] != 0, "Memory size should not be zero. ");
    return buf[6];
}

int mbox_get_clock_rate(){
    __attribute__((aligned(16)))
    uint32_t buf[] = {36, 0, MBOX_TAG_GET_CLOCK_RATE, 8, 0, 1, 0, MBOX_TAG_END};
    asserts((VA2PA(buf) & 0xF) == 0, "Buffer should align to 16 bytes. ");

    disb();
    dccivac(buf, sizeof(buf));
    disb();
    mbox_write(VA2PA(buf), 8);
    disb();
    mbox_read(8);
    disb();
    dccivac(buf, sizeof(buf));
    disb();

    cprintf("- clock rate: %d\n", buf[6]);
    return buf[6];
}

