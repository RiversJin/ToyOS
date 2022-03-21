#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "include/stdint.h"
#include "arch/aarch64/board/raspi3/uart.h"
#include "sync/spinlock.h"
#include "proc/proc.h"
#include "include/param.h"
#include "file/file.h"

#define BACKSPACE 0x100
#define C(x) ((x) - '@')  // 控制字符

extern void uart_putchar(int c);
extern void uart_putchar_sync(int c);
extern void uart_init();


void consputc(int c){
    if(c == BACKSPACE){
        uart_putchar_sync('\b');
        uart_putchar_sync(' ');
        uart_putchar_sync('\b');
    }else{
        uart_putchar_sync(c);
    }
}

struct cons{
    struct spinlock lock;
    char buf[INPUT_BUF];
    uint32_t r; // read index
    uint32_t w; // write index
    uint32_t e; // edit index
} cons;

int consolewrite(char* src, int n){
    int i;
    for(i = 0;i < n; ++i){
        uart_putchar(*(src + i));
    }
    return i;
}

int consoleread(char* dst, int n){
    int c;
    uint32_t target =  n;
    acquire_spin_lock(&cons.lock);
    while(n > 0){
        while(cons.r == cons.w){
            if(myproc()->killed){
                release_spin_lock(&cons.lock);
                return -1;
            }
            sleep(&cons.r, &cons.lock);
        }
        c = cons.buf[cons.r ++ % INPUT_BUF];
        if(c == C('D')){
            // end of file
            if(n < target ){
                cons.r -- ;
            }
            break;
        }
        *dst = c;
        dst++;
        --n;
        if(c == '\n'){ 
            break;
        }
    }
    release_spin_lock(&cons.lock);
    return target - n;
}

void consoleintr(int c){
    acquire_spin_lock(&cons.lock);
    switch(c){
        case C('P'):
            // procdump
            break;

        case C('U'):
            while(cons.e != cons.w && cons.buf[(cons.e - 1) % INPUT_BUF] != '\n'){
                cons.e --;
                consputc(BACKSPACE);
            }
            break;
        case C('H'): // 退格键
        case '\x7f': 
            if(cons.e != cons.w){
                cons.e -- ;
                consputc(BACKSPACE);
            }
            break;
        default:
            if(c != 0 && cons.e - cons.r < INPUT_BUF){
                c = (c == '\r') ? '\n' : c;
                consputc(c);
                cons.buf[cons.e ++ % INPUT_BUF] = c;
                if(c == '\n' || c == C('D') || cons.e == cons.r + INPUT_BUF){
                    // 如果到达行尾的话调用 consoleread
                    cons.w = cons.e;
                    wakeup(&cons.r);
                }
            }
            break;
    }
    release_spin_lock(&cons.lock);
}
extern struct devsw devsw[NDEV];
void consoleinit(void){
    init_spin_lock(&cons.lock,"console");
    uart_init();
    // 配置终端设备驱动
    devsw[CONSOLE].read = consoleread;
    devsw[CONSOLE].write = consolewrite;
}
