#include <stdarg.h>
#include "include/stdint.h"
#include <stdbool.h>
#include "sync/spinlock.h"
#include "console.h"

volatile int panicked = 0;
static struct print_lock{
    struct spinlock lock;
    bool locking;
} print_lock;
static char digits[] = "0123456789ABCEDF";
/**
 * @brief 向串口打印数字
 * 
 * @param x 数字的值
 * @param base 进制
 * @param sign 是否有符号
 */
static void printint(int64_t x,int base, int sign){
    static char buf[64];
    if(sign && (sign = x < 0)){
        x = -x;
    }
    int i = 0;
    uint64_t t = x;
    do {
        buf[i++] = digits[t % base];
    } while ((t /= base) != 0);
    if(sign){
        buf[i++] = '-';
    }
    while (i-- >= 0){
        consputc(buf[i]);
    }
}
static void printptr(uint64_t x){
    consputc('0');
    consputc('x');
    for(int i = 0; i < (sizeof(uint64_t)*2); ++i, x <<= 4){
        consputc(digits[x >> (sizeof(uint64_t)*8 - 4)]);
    }
}

static void vprintfmt(void (*putch)(int), const char *fmt, va_list ap){
    int i, c;
    char *s;
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            putch(c);
            continue;
        }

        int l = 0;
        for (; fmt[i+1] == 'l'; i++)
            l++;

        if (!(c = fmt[++i] & 0xff))
            break;

        switch (c) {
        case 'u':
            if (l == 2) printint(va_arg(ap, int64_t), 10, 0);
            else printint(va_arg(ap, int), 10, 0);
            break;
        case 'd':
            if (l == 2) printint(va_arg(ap, int64_t), 10, 1);
            else printint(va_arg(ap, int), 10, 1);
            break;
        case 'x':
            if (l == 2) printint(va_arg(ap, int64_t), 16, 0);
            else printint(va_arg(ap, int), 16, 0);
            break;
        case 'p':
            printptr((int64_t)va_arg(ap, void *));
            break;
        case 'c':
            putch(va_arg(ap, int));
            break;
        case 's':
            if ((s = (char*)va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                putch(*s);
            break;
        case '%':
            putch('%');
            break;
        default:
            /* Print unknown % sequence to draw attention. */
            putch('%');
            putch(c);
            break;
        }
    }
}
void panic(const char *fmt, ...);

__attribute__((format(printf,1,2)))
void cprintf(const char *fmt, ...){
    bool locking = print_lock.locking;
    if(locking)
        acquire_spin_lock(&print_lock.lock);
    if(fmt == NULL){
        panic("%s: fmt is NULL",__FUNCTION__);
    }
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(consputc, fmt,ap);
    va_end(ap);
    if(locking){
        release_spin_lock(&print_lock.lock);
    }
}
__attribute__((format(printf,1,2)))
__attribute__((noreturn))
void panic(const char *fmt, ...){
    print_lock.locking = false;
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(consputc, fmt,ap);
    va_end(ap);
    cprintf("\t%s:%d: kernel panic.\n", __FILE__, __LINE__);
    panicked = true;
    while(1);  
}

void printfinit(void){
    init_spin_lock(&print_lock.lock, "pr");
    print_lock.locking = 1;
}