#include <stdarg.h>
#include <stdint.h>

#include "arch/aarch64/board/raspi3/uart.h"

extern void uart_putchar(int c);
extern void uart_init();

/**
 * @brief 初始化中断 在这里就是初始化串口
 * 
 */
void console_init(void){
    uart_init();
}
/**
 * @brief 向串口打印数字
 * 
 * @param x 数字的值
 * @param base 进制
 * @param sign 是否有符号
 */
static void printint(int64_t x,int base, int sign){
    static char digit[] = "0123456789ABCEDF";
    static char buf[64];
    if (sign && x < 0) {
        x = -x;
        uart_putchar('-');
    }
    int i = 0;
    uint64_t t = x;
    do {
        buf[i++] = digit[t % base];
    } while (t /= base);
    while (i--) uart_putchar(buf[i]);
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
            printint((int64_t)va_arg(ap, void *), 16, 0);
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
__attribute__((format(printf,1,2)))
void cprintf(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(uart_putchar, fmt,ap);
    va_end(ap);
}
__attribute__((format(printf,1,2)))
__attribute__((noreturn))
void panic(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(uart_putchar, fmt,ap);
    va_end(ap);
    cprintf("\t%s:%d: kernel panic.\n", __FILE__, __LINE__);
    while(1);  
}