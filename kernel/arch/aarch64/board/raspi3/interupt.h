#ifndef INTERUPT_H
#define INTERUPT_H
#include "local_peripherals.h"

#define IRQ_BASIC_PENDING       (MMIO_BASE + 0xB200)
#define IRQ_PENDING_1           (MMIO_BASE + 0xB204)
#define IRQ_PENDING_2           (MMIO_BASE + 0xB208)
#define FIQ_CONTROL             (MMIO_BASE + 0xB20C)
#define ENABLE_IRQS_1           (MMIO_BASE + 0xB210)
#define ENABLE_IRQS_2           (MMIO_BASE + 0xB214)
#define ENABLE_BASIC_IRQS       (MMIO_BASE + 0xB218)
#define DISABLE_IRQS_1          (MMIO_BASE + 0xB21C)
#define DISABLE_IRQS_2          (MMIO_BASE + 0xB220)
#define DISABLE_BASIC_IRQS      (MMIO_BASE + 0xB224)

#define AUX_INT                 (1 << 29)

/* IRQ Source Definitions */

#define IRQ_CNTPNSIRQ           (1 << 1)
#define IRQ_TIMER               (1 << 11) /* local timer (unused) */
#define IRQ_GPU                 (1 << 8)

#endif /* INTERUPT_H */