#ifndef EXCEPTION_H
#define EXCEPTION_H

#define BAD_SYNC_SP0    0
#define BAD_IRQ_SP0     1
#define BAD_FIQ_SP0     2
#define BAD_ERROR_SP0   3

#define BAD_FIQ_SPx     4
#define BAD_ERROR_SPx   5

#define BAD_FIQ_EL0     6
#define BAD_ERROR_EL0   7

#define BAD_AARCH32     8


/* 在ESR_EL1(Exception Syndrome Register)中, Exception Class的定义 */
#define EC_Unknown  0x0
#define EC_WF_INSTRUCTION   0x0
#define EC_ILLEGAL_EXECUTION_STATE  0xE
#define EC_SVC64    0x15

#define ISS_MASK    0xFFFFFF
#endif