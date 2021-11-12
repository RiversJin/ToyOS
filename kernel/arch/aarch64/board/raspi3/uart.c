#include <stdarg.h>
#include <stdint.h>

#include "uart.h"
#include "gpio.h"
#include "../../arm.h"
/**
 * @brief 初始化串口
 * 
 */
void uart_init(void)
{
    uint32_t selector;
    selector = get32(GPFSEL1);
    selector &= ~(7 << 12); /* Clean gpio14 */
    selector |= 2 << 12;    /* Set alt5 for gpio14 */
    selector &= ~(7 << 15); /* Clean gpio15 */
    selector |= 2 << 15;    /* Set alt5 for gpio15 */
    put32(GPFSEL1, selector);

    put32(GPPUD, 0);
    delay(150);
    put32(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    put32(GPPUDCLK0, 0);

    put32(AUX_ENABLES, 1);       // 使能mini_uart
    put32(AUX_MU_CNTL_REG, 0);   // 禁用串口的流控制
    put32(AUX_MU_IER_REG, 0);    // 先关掉串口传输中断,一会要配置串口
    put32(AUX_MU_LCR_REG, 3);    // 设置串口8bit模式
    put32(AUX_MU_MCR_REG, 0);    // 设置RTS永远为高电平
    put32(AUX_MU_BAUD_REG, 270); // 设置波特率为115200
    put32(AUX_MU_CNTL_REG, 3);   // 重新使能串口传输
}
/**
 * @brief 向串口发送一个字符
 * 
 * @param c 
 */
void uart_putchar(int c)
{
    while (!(get32(AUX_MU_LSR_REG) & 0x20))
        ;
    put32(AUX_MU_IO_REG, c & 0xff);
}
/**
 * @brief 从串口读取一个字符
 * 
 * @return char 
 */
char uart_getchar(void)
{
    while (!(get32(AUX_MU_LSR_REG) & 0x01))
        ;
    return get32(AUX_MU_IO_REG) & 0xff;
}