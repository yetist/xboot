/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <uart/uart.h>

#define UART_BASE_ADDR    (0x8000ULL <<48 | 0x1fe001e0)

#define UART_REG_LSR      0x5       // 线路状态寄存器
#define UART_REG_TXFIFO   0x00
#define UART_REG_RXFIFO   0x00

#define LSR_DR            (1<<0)    // 接收数据有效位, 1为有数据
#define LSR_TFE           (1<<5)    // 传输FIFO位空表示位, 1为空


struct uart_ls7a_pdata_t {
  virtual_addr_t virt;
};

static bool_t uart_ls7a_set(struct uart_t * uart, int baudrate, int data, int parity, int stop)
{
  /* QEMU 虚拟串口下默认透传，直接返回 TRUE 告知配置成功 */
  return TRUE;
}

static bool_t uart_ls7a_get(struct uart_t * uart, int * baudrate, int * data, int * parity, int * stop)
{
  if (baudrate) *baudrate = 115200;
  if (data)     *data = 8;
  if (parity)   *parity = 0;
  if (stop)     *stop = 1;
  return TRUE;
}

static ssize_t uart_ls7a_read(struct uart_t * uart, u8_t * buf, size_t n)
{
  struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;
  size_t i;

  for (i = 0; i < n; i++) {
    /* 检查 LSR 寄存器第 0 位（DR），没有键盘输入数据则立刻打破 */
    if ((read8(UART_BASE_ADDR + UART_REG_LSR) & LSR_DR) == 0) {
      break;
    }
    buf[i] = read8(UART_BASE_ADDR + UART_REG_RXFIFO);
  }
  return i;
}

static ssize_t uart_ls7a_write(struct uart_t * uart, const u8_t * buf, size_t n)
{
  struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;
  size_t i;

  for (i = 0; i < n; i++) {
    while ((read8(UART_BASE_ADDR + UART_REG_LSR) & LSR_TFE) == 0);
    write8(UART_BASE_ADDR + UART_REG_TXFIFO, buf[i]);
  }

  return n;
}

static struct device_t * uart_ls7a_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct uart_ls7a_pdata_t * pdat;
  struct uart_t * uart;
  struct device_t * dev;
  virtual_addr_t virt = phys_to_virt(dt_read_address(n));

  pdat = malloc(sizeof(struct uart_ls7a_pdata_t));
  if (!pdat)
    return NULL;

  uart = malloc(sizeof(struct uart_t));
  if (!uart) {
    free(pdat);
    return NULL;
  }

  pdat->virt = virt;
  /* ls7a 硬件寄存器初始化 */
  write8(pdat->virt + 3, 0x83);
  write8(pdat->virt + 0, 0x03);
  write8(pdat->virt + 1, 0x00);
  write8(pdat->virt + 3, 0x03);
  write8(pdat->virt + 1, 0x00);
  write8(pdat->virt + 2, 0x01);

  uart->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  uart->set = uart_ls7a_set;
  uart->get = uart_ls7a_get;
  uart->read = uart_ls7a_read;
  uart->write = uart_ls7a_write;
  uart->priv = pdat;

  if (! (dev = register_uart(uart, drv))) {
    free_device_name(uart->name);
    free(uart->priv);
    free(uart);
    return NULL;
  }

  return dev;
}

static void uart_ls7a_remove(struct device_t * dev)
{
  struct uart_t * uart = (struct uart_t *)dev->priv;

  if (uart) {
    unregister_uart(uart);
    free_device_name(uart->name);
    free(uart->priv);
    free(uart);
  }
}

static void uart_ls7a_suspend(struct device_t * dev)
{
}

static void uart_ls7a_resume(struct device_t * dev)
{
}

static struct driver_t uart_ls7a = {
  .name	= "uart-ls7a",
  .probe	= uart_ls7a_probe,
  .remove	= uart_ls7a_remove,
  .suspend = uart_ls7a_suspend,
  .resume  = uart_ls7a_resume,
};

static __init void uart_ls7a_driver_init(void)
{
  register_driver(&uart_ls7a);
}

static __exit void uart_ls7a_driver_exit(void)
{
  unregister_driver(&uart_ls7a);
}

driver_initcall(uart_ls7a_driver_init);
driver_exitcall(uart_ls7a_driver_exit);
