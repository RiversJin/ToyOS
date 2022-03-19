#ifndef LOG_H
#define LOG_H

#include "fs.h"
#include "../buffer/buf.h"

void initlog(int,struct superblock*);
void log_write(struct buf*);
void begin_op(void);
void end_op(void);

#endif // LOG_H