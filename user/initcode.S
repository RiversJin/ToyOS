#include "../kernel/syscall/syscall.h"
# exec(init, argv)
.global start
start:
    ldr x0, =cmd
    ldr x1, =argv
    mov x8, #SYS_exec
    svc 0x0
    ldr x4, =start
    br x4
cmd:
    .string "/init\0"
argv:
    .word cmd
    .word 0
