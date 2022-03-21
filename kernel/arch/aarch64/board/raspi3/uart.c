#include <stdarg.h>
#include "../../../../include/stdint.h"
#include "../../../../proc/proc.h"

#include "uart.h"
#include "gpio.h"
#include "../../arm.h"
#include "sync/spinlock.h"

extern volatile int panicked;
static struct spinlock uart_tx_lock;
#define UART_TXBUF_SIZE 32
char uart_tx_buf[UART_TXBUF_SIZE];
uint64_t uart_tx_w; // 下一个写入uart_tx_buf[uart_tx_w % UART_TXBUF_SIZE]
uint64_t uart_tx_r; // 下一个读 uart_tx_buf[uart_tx_r % UART_TXBUF_SIZE]
static void uart_start(void);
/**
 * @brief 初始化串口
 * 
 */

void uart_init(void)
{
    /*
    uint32_t selector;
    selector = get32(GPFSEL1);
    selector &= ~(7 << 12); // Clean gpio14 
    selector |= 2 << 12;    // Set alt5 for gpio14 
    selector &= ~(7 << 15); // Clean gpio15 
    selector |= 2 << 15;    // Set alt5 for gpio15 
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
    put32(AUX_MU_IIR_REG,((0b11<<1)|(0b11 << 6))); // 清空接受和发送的FIFO 使能FIFO
    put32(AUX_MU_CNTL_REG, 3);   // 重新使能串口传输
    put32(AUX_MU_IER_REG, 0b11); // 打开串口收发中断 
    */
   uint32_t selector, enables;

    /* initialize UART */
    enables = get32(AUX_ENABLES);
    enables |= 1;
    put32(AUX_ENABLES, enables); /* enable UART1, AUX mini uart */
    put32(AUX_MU_CNTL_REG, 0);
    put32(AUX_MU_LCR_REG, 3); /* 8 bits */
    put32(AUX_MU_MCR_REG, 0);
    put32(AUX_MU_IER_REG, 3 << 2 | 1);
    put32(AUX_MU_IIR_REG, 0xc6); /* disable interrupts */
    put32(AUX_MU_BAUD_REG, 270); /* 115200 baud */
    /* map UART1 to GPIO pins */
    selector = get32(GPFSEL1);
    selector &= ~((7 << 12) | (7 << 15)); /* gpio14, gpio15 */
    selector |= (2 << 12) | (2 << 15);    /* alt5 */
    put32(GPFSEL1, selector);

    put32(GPPUD, 0); /* enable pins 14 and 15 */
    delay(150);
    put32(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    put32(GPPUDCLK0, 0);       /* flush GPIO setup */
    put32(AUX_MU_CNTL_REG, 3); /* enable Tx, Rx */
    
    init_spin_lock(&uart_tx_lock,"uart_tx_lock");
}
/**
 * @brief 向串口发送一个字符 异步
 * 
 * @param c 
 */
void uart_putchar(int c)
{
    acquire_spin_lock(&uart_tx_lock);
    if(panicked){
        while(1);
    }
    while(1){
        if(uart_tx_w == uart_tx_r + UART_TXBUF_SIZE){
            // 此时buffer已满 等待
            sleep(&uart_tx_r, &uart_tx_lock);
        } else {
            uart_tx_buf[uart_tx_w % UART_TXBUF_SIZE] = c;
            uart_tx_w += 1;
            uart_start();
            release_spin_lock(&uart_tx_lock);
            return;
        }
    }
}
/**
 * @brief 向串口发送一个字符 同步
 * 
 * @param c 
 */
void uart_putchar_sync(int c){
    push_off();
    if(panicked){ 
        while(1);
    }

    while((get32(AUX_MU_LSR_REG) & LSR_TX_IDLE) == 0); // 等待发送队列可用
    put32(AUX_MU_IO_REG,c);
    pop_off();
}
/**
 * @brief 若uart空闲并且队列中有字符等待发送 那么发送这个字符
 * 调用者需持有uart_tx_lock
 * 
 */
void uart_start(){
    while(1){
        if(uart_tx_w == uart_tx_r){ 
            return; // 队列为空 直接退出即可 
        }   
        if((get32(AUX_MU_LSR_REG) & LSR_TX_IDLE) == 0){
            return; // 如果此时uart模块忙 那么也没办法传输
        }
        int c = uart_tx_buf[uart_tx_r % UART_TXBUF_SIZE];
        uart_tx_r += 1;
        wakeup(&uart_tx_r); // 如果有进程等待buffer 那么现在可以开始表演了
        put32(AUX_MU_IO_REG, c);
    }
    
}
/**
 * @brief 从串口读取一个字符
 * 
 * @return char 
 */
int uart_getchar(void){
    if(get32(AUX_MU_LSR_REG) & LSR_RX_READY){
        // 如果有数据 读取
        return get32(AUX_MU_IO_REG);
    } else {
        // 否则返回失败
        return -1;
    }
}

extern void consoleintr(int c);

void uartintr(void){
    while(1){
        int c = uart_getchar();
        if( c == -1){
            break;
        }
        consoleintr(c);
    }

    acquire_spin_lock(&uart_tx_lock);
    uart_start();
    release_spin_lock(&uart_tx_lock);
}