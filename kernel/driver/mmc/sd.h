#ifndef SD_H
#define SD_H

#include "buffer/buf.h"

#define SD_OK              0
#define SD_ERROR           1
#define SD_TIMEOUT         2
#define SD_BUSY            3
#define SD_NO_RESP         5
#define SD_ERROR_RESET     6
#define SD_ERROR_CLOCK     7
#define SD_ERROR_VOLTAGE   8
#define SD_ERROR_APP_CMD   9
#define SD_CARD_CHANGED    10
#define SD_CARD_ABSENT     11
#define SD_CARD_REINSERTED 12

#define SD_READ_BLOCKS  0
#define SD_WRITE_BLOCKS 1

void sd_init();
void sd_intr();
void sd_rw(struct buf*);
void sd_test();

#endif /* SD_H */