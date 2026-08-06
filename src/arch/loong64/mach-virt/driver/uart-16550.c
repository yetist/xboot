/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <uart/uart.h>

struct uart_16550_pdata_t {
  virtual_addr_t virt;
};

static bool_t uart_16550_set(struct uart_t * uart, int baudrate, int data, int parity, int stop)
{
  /* QEMU 虚拟串口下默认透传，直接返回 TRUE 告知配置成功 */
  return TRUE;
}

static bool_t uart_16550_get(struct uart_t * uart, int * baudrate, int * data, int * parity, int * stop)
{
  if (baudrate) *baudrate = 115200;
  if (data)     *data = 8;
  if (parity)   *parity = 0;
  if (stop)     *stop = 1;
  return TRUE;
}

static ssize_t uart_16550_read(struct uart_t * uart, u8_t * buf, size_t n)
{
  struct uart_16550_pdata_t * pdat = (struct uart_16550_pdata_t *)uart->priv;
  size_t i;

  for (i = 0; i < n; i++) {
    /* 检查 LSR 寄存器第 0 位（DR），没有键盘输入数据则立刻打破 */
    if ((read8(pdat->virt + 5) & 0x01) == 0) {
      break;
    }
    buf[i] = read8(pdat->virt + 0);
  }
  return i;
}

static ssize_t uart_16550_write(struct uart_t * uart, const u8_t * buf, size_t n)
{
  struct uart_16550_pdata_t * pdat = (struct uart_16550_pdata_t *)uart->priv;
  size_t i;

  for (i = 0; i < n; i++) {
    while ((read8(pdat->virt + 5) & 0x20) == 0);
    write8(pdat->virt + 0, buf[i]);
  }

  return n;
}

static struct device_t * uart_16550_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct uart_16550_pdata_t * pdat;
  struct uart_t * uart;
  struct device_t * dev;
  virtual_addr_t virt = phys_to_virt(dt_read_address(n));

  pdat = malloc(sizeof(struct uart_16550_pdata_t));
  if (!pdat)
    return NULL;

  uart = malloc(sizeof(struct uart_t));
  if (!uart) {
    free(pdat);
    return NULL;
  }

  pdat->virt = virt;
  /* 16550 硬件寄存器初始化 */
  write8(pdat->virt + 3, 0x83);
  write8(pdat->virt + 0, 0x03);
  write8(pdat->virt + 1, 0x00);
  write8(pdat->virt + 3, 0x03);
  write8(pdat->virt + 1, 0x00);
  write8(pdat->virt + 2, 0x01);

  uart->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  uart->set = uart_16550_set;
  uart->get = uart_16550_get;
  uart->read = uart_16550_read;
  uart->write = uart_16550_write;
  uart->priv = pdat;

  if (! (dev = register_uart(uart, drv))) {
	free_device_name(uart->name);
    free(uart->priv);
    free(uart);
    return NULL;
  }

  return dev;
}

static void uart_16550_remove(struct device_t * dev)
{
  struct uart_t * uart = (struct uart_t *)dev->priv;

  if (uart) {
    unregister_uart(uart);
	free_device_name(uart->name);
    free(uart->priv);
    free(uart);
  }
}

static void uart_16550_suspend(struct device_t * dev)
{
}

static void uart_16550_resume(struct device_t * dev)
{
}

static struct driver_t uart_16550 = {
  .name	= "uart-16550",
  .probe	= uart_16550_probe,
  .remove	= uart_16550_remove,
  .suspend = uart_16550_suspend,
  .resume  = uart_16550_resume,
};

static __init void uart_16550_driver_init(void)
{
  register_driver(&uart_16550);
}

static __exit void uart_16550_driver_exit(void)
{
  unregister_driver(&uart_16550);
}

driver_initcall(uart_16550_driver_init);
driver_exitcall(uart_16550_driver_exit);
