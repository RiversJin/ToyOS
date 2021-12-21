#include "bcm2837_sd.h"
#include "arch/aarch64/board/raspi3/peripherals_base.h"
/* 地址定义 参考 BCM2837-Broadcom.pdf page 66 */

#define EMMC_REG_BASE   (MMIO_BASE + 0x300000)

#define EMMC_ARG2 (EMMC_REG_BASE + 0x0)
#define EMMC_BLKSIZECNT (EMMC_REG_BASE + 0x4) // block size and count
#define EMMC_ARG1 (EMMC_REG_BASE + 0x8)
#define EMMC_CMDTM (EMMC_REG_BASE + 0xC) // command and Transfer Mode
#define EMMC_RESP0 (EMMC_REG_BASE + 0x10) // response bits 31:0
#define EMMC_RESP1 (EMMC_REG_BASE + 0x14) // response bits 63:32
#define EMMC_RESP2 (EMMC_REG_BASE + 0x18) // response bits 95:64
#define EMMC_RESP3 (EMMC_REG_BASE + 0x1C) // response bits 127:64
#define EMMC_DATA (EMMC_REG_BASE + 0x20)
#define EMMC_STATUS (EMMC_REG_BASE + 0x24)
#define EMMC_CONTROL0 (EMMC_REG_BASE + 0x28) // host control bits
#define EMMC_CONTROL1 (EMMC_REG_BASE + 0x2C) // host control bits
#define EMMC_INTERRUPT (EMMC_REG_BASE + 0x30) // interrupt flags
#define EMMC_IRPT_MASK (EMMC_REG_BASE + 0x34) // interrupt enable
#define EMMC_IRPT_EN (EMMC_REG_BASE + 0x38) // interrupt generation enable
#define EMMC_CONTROL2 (EMMC_REG_BASE + 0x3C)
#define EMMC_FORCE_IRPT (EMMC_REG_BASE + 0x50) // force interrupt event
#define EMMC_BOOT_TIMEOUT (EMMC_REG_BASE + 0x70) // timeout in boot mode
#define EMMC_DBG_SEL (EMMC_REG_BASE + 0x74) // debug bus configuration
#define EMMC_EXRDFIFO_CFG (EMMC_REG_BASE + 0x80) // extension fifo configuration
#define EMMC_EXRDFIFO_EN (EMMC_REG_BASE + 0x84) // extension fifo enable
#define EMMC_TUNE_STEP_STD (EMMC_REG_BASE + 0x8C) // card clock tuning steps for SDR
#define EMMC_TUNE_STEP_DDR (EMMC_REG_BASE + 0x90) // card clock tuning steps for DDR
#define EMMC_SPI_INT_SPT (EMMC_REG_BASE + 0xF0) // SPI interrupt support
#define EMMC_SLOTISR (EMMC_REG_BASE + 0xFC) // Slot Interrupt Status and Version
