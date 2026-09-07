/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <clk/clk.h>
#include <reset/reset.h>
#include <gpio/gpio.h>
#include <uart/uart.h>

#define UART_RCV_BUFFER         0
#define UART_SEND_BUFFER        0

#define UART_INTR_ENABLE_REG    0x01
#define UART_INTR_ID_REG        0x02
#define UART_FIFO_CONTROL_REG   0x02
#define UART_LINE_CONTROL_REG   0x03
#define UART_MODEM_CONTROL_REG  0x04
#define UART_LINE_STATUS_REG    0x05

#define UART_DIVISOR_LSB        0x00
#define UART_DIVISOR_MSB        0x01

#define LSR_DR                  (1<<0)    // 接收数据有效位, 1为有数据
#define LSR_TFE                 (1<<5)    // 传输FIFO位空表示位, 1为空


struct uart_ls7a_pdata_t {
  virtual_addr_t virt;

	char * clk;
	int reset;
	int txd;
	int txdcfg;
	int rxd;
	int rxdcfg;
	int baud;
	int data;
	int parity;
	int stop;
};

static bool_t uart_ls7a_set(struct uart_t * uart, int baudrate, int data, int parity, int stop)
{
  struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;
	u8_t dreg, preg, sreg;
	u32_t val, udiv;

	if(baudrate < 0)
		return FALSE;
	if((data < 5) || (data > 8))
		return FALSE;
	if((parity < 0) || (parity > 1))
		return FALSE;
	if((stop < 0) || (stop > 2))
		return FALSE;

	switch(data)
	{
	case 5:	/* Data bits = 5 */
		dreg = 0x0;
		break;
	case 6:	/* Data bits = 6 */
		dreg = 0x1;
		break;
	case 7:	/* Data bits = 7 */
		dreg = 0x2;
		break;
	case 8:	/* Data bits = 8 */
		dreg = 0x3;
		break;
	default:
		return FALSE;
	}

	switch(parity)
	{
	case 0:	/* Parity odd 奇数 */
		preg = 0x0;
		break;
	case 1:	/* Parity even 偶数 */
		preg = 0x1;
		break;
	default:
		return FALSE;
	}

	switch(stop)
	{
	case 1:	/* Stop bits = 1 */
		sreg = 0;
		break;
	case 2:	/* Stop bits = 2 */
		sreg = 3;
		break;
	case 0:	/* Stop bits = 1.5 */
	default:
		return FALSE;
	}

	pdat->baud = baudrate;
	pdat->data = data;
	pdat->parity = parity;
	pdat->stop = stop;

  { // 访问操作分频锁存器
    val = read8(pdat->virt + UART_LINE_CONTROL_REG);
    val |= (1 << 7);
    write8(pdat->virt + UART_LINE_CONTROL_REG, val);

    udiv = clk_get_rate(pdat->clk) / (16 * baudrate);
    write8(pdat->virt + UART_DIVISOR_LSB, udiv & 0xff);
    write8(pdat->virt + UART_DIVISOR_MSB, (udiv >> 8) & 0xff);

    // 访问操作正常寄存器
    val = read8(pdat->virt + UART_LINE_CONTROL_REG);
    val &= ~(1 << 7);
    write8(pdat->virt + UART_LINE_CONTROL_REG, val);
  }

	val = read8(pdat->virt + UART_LINE_CONTROL_REG);
	val &= ~0x17;
	val |= (dreg << 0) | (sreg << 2) | (preg << 4);
	write8(pdat->virt + UART_LINE_CONTROL_REG, val);

  return TRUE;
}

static bool_t uart_ls7a_get(struct uart_t * uart, int * baudrate, int * data, int * parity, int * stop)
{
	struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;

  if (baudrate)
    *baudrate = pdat->baud;
  if (data)
    *data = pdat->data;
  if (parity)
    *parity = pdat->parity;
  if (stop)
    *stop = pdat->stop;
  return TRUE;
}

static ssize_t uart_ls7a_read(struct uart_t * uart, u8_t * buf, size_t n)
{
  size_t i;
  struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;

  for (i = 0; i < n; i++) {
    if ((read8(pdat->virt + UART_LINE_STATUS_REG) & LSR_DR) == 0) {
      break;
    }
    buf[i] = read8(pdat->virt + UART_RCV_BUFFER);
  }
  return i;
}

static ssize_t uart_ls7a_write(struct uart_t * uart, const u8_t * buf, size_t n)
{
  size_t i;
  struct uart_ls7a_pdata_t * pdat = (struct uart_ls7a_pdata_t *)uart->priv;

  for (i = 0; i < n; i++) {
    while ((read8(pdat->virt + UART_LINE_STATUS_REG) & LSR_TFE) == 0);
    write8(pdat->virt + UART_SEND_BUFFER, buf[i]);
  }

  return n;
}

static struct device_t * uart_ls7a_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct uart_ls7a_pdata_t * pdat;
  struct uart_t * uart;
  struct device_t * dev;
  virtual_addr_t virt = phys_to_virt(dt_read_address(n));
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

  pdat = malloc(sizeof(struct uart_ls7a_pdata_t));
  if (!pdat)
    return NULL;

  uart = malloc(sizeof(struct uart_t));
  if (!uart) {
    free(pdat);
    return NULL;
  }

  pdat->virt = virt;
	pdat->clk = strdup(clk);
	pdat->reset = dt_read_int(n, "reset", -1);
	pdat->txd = dt_read_int(n, "txd-gpio", -1);
	pdat->txdcfg = dt_read_int(n, "txd-gpio-config", -1);
	pdat->rxd = dt_read_int(n, "rxd-gpio", -1);
	pdat->rxdcfg = dt_read_int(n, "rxd-gpio-config", -1);
	pdat->baud = dt_read_int(n, "baud-rates", 115200);
	pdat->data = dt_read_int(n, "data-bits", 8);
	pdat->parity = dt_read_int(n, "parity-bits", 0);
	pdat->stop = dt_read_int(n, "stop-bits", 1);

  uart->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  uart->set = uart_ls7a_set;
  uart->get = uart_ls7a_get;
  uart->read = uart_ls7a_read;
  uart->write = uart_ls7a_write;
  uart->priv = pdat;

	clk_enable(pdat->clk);
	if(pdat->reset >= 0)
		reset_deassert(pdat->reset);
	if(pdat->txd >= 0)
	{
		if(pdat->txdcfg >= 0)
			gpio_set_cfg(pdat->txd, pdat->txdcfg);
		gpio_set_pull(pdat->txd, GPIO_PULL_UP);
	}
	if(pdat->rxd >= 0)
	{
		if(pdat->rxdcfg >= 0)
			gpio_set_cfg(pdat->rxd, pdat->rxdcfg);
		gpio_set_pull(pdat->rxd, GPIO_PULL_UP);
	}

	write8(pdat->virt + UART_INTR_ENABLE_REG, 0x0);
	write8(pdat->virt + UART_FIFO_CONTROL_REG, (0x1<<6 | 0b110)); // TL[7:6]=0b01, Txset[2]=1, Rxset[1]=1
	write8(pdat->virt + UART_MODEM_CONTROL_REG, 0x0);

	uart_ls7a_set(uart, pdat->baud, pdat->data, pdat->parity, pdat->stop);

  if (! (dev = register_uart(uart, drv))) {
		clk_disable(pdat->clk);
		free(pdat->clk);
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
