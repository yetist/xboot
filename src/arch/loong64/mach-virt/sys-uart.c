/*
 * sys-uart.c
 */

#include <xboot.h>

#define UART_BASE_ADDR    (0x8000ULL <<48 | 0x1fe001e0)

#define UART_REG_LSR      0x5       // 线路状态寄存器
#define UART_REG_TXFIFO   0x00
#define UART_REG_RXFIFO   0x00

#define LSR_DR            (1<<0)    // 接收数据有效位, 1为有数据
#define LSR_TFE           (1<<5)    // 传输FIFO位空表示位, 1为空

void sys_uart_init(void)
{
}

void uart_putchar(char ch) {
  while(!(read8(UART_BASE_ADDR + UART_REG_LSR) & LSR_TFE));
  write8(UART_BASE_ADDR + UART_REG_TXFIFO, ch);
}

char uart_getchar(void) {
  while(!(read8(UART_BASE_ADDR + UART_REG_LSR) & LSR_DR));
  return read8(UART_BASE_ADDR + UART_REG_RXFIFO);
}

void uart_puts(const char *str) {
  while (*str) {
    if (*str == '\n')
      uart_putchar('\r');
    uart_putchar(*str++);
  }
}
