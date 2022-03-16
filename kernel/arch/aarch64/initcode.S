#include "../../syscall/syscall.h"

.global user_init_begin
.global user_init_end

.p2align 4
# exit(0);
user_init_begin:
mov     x0, 0
mov     x8, #SYS_exit
svc     0x0
b user_init_begin
user_init_end: