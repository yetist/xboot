/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <xboot.h>
#include <clk/clk.h>
#include <interrupt/interrupt.h>
#include <clockevent/clockevent.h>

#define LOONGARCH_CSR_TCFG      0x417   /* 定时器配置寄存器 */
#define LOONGARCH_CSR_TVAL      0x418   /* 定时器倒计时值寄存器 */
#define LOONGARCH_CSR_TICLR     0x419   /* 定时器中断清除寄存器 */

#define CSR_TCFG_ENABLE         (1ULL << 0)  /* 使能定时器 */
#define CSR_TCFG_PERIODIC       (1ULL << 1)  /* 周期触发, 0:单次(One-shot)触发 */

struct ce_loongarch_timer_pdata_t {
  char * clk;
  int irq;
};

static int ce_loongarch_timer_next_event(struct clockevent_t * ce, u64_t evt)
{
  csr_write(LOONGARCH_CSR_TCFG, (CSR_TCFG_ENABLE | CSR_TCFG_PERIODIC | evt <<2 ));

  return 0;
}

static void ce_loongarch_timer_interrupt(void * data)
{
  struct clockevent_t * ce = (struct clockevent_t *)data;

  csr_write(LOONGARCH_CSR_TICLR, 1ULL);

  ce->handler(ce, ce->data);
}

static struct device_t * ce_loongarch_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct ce_loongarch_timer_pdata_t * pdata;
  struct clockevent_t * ce;
  struct device_t * dev;

  char * clk = dt_read_string(n, "clock-name", NULL);

  if(!search_clk(clk))
    return NULL;

  int irq = dt_read_int(n, "interrupt-number", 11);

  pdata = malloc(sizeof(struct ce_loongarch_timer_pdata_t));
  if (!pdata)
    return NULL;

  ce = malloc(sizeof(struct clockevent_t));
  if(!ce)
  {
    free(pdata);
    return NULL;
  }

  pdata->clk = strdup(clk);
  pdata->irq = irq;
  clk_enable(pdata->clk);
  clockevent_calc_mult_shift(ce, clk_get_rate(pdata->clk), 10);

  ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
  ce->max_delta_ns = clockevent_delta2ns(ce, 0xffffffffffffffff);
  ce->next = ce_loongarch_timer_next_event;
  ce->priv = pdata;

  request_irq(pdata->irq, ce_loongarch_timer_interrupt, IRQ_TYPE_NONE, ce);

  if(!(dev = register_clockevent(ce, drv)))
  {
    clk_disable(pdata->clk);
    free(pdata->clk);
    free_device_name(ce->name);
    free(ce->priv);
    free(ce);
    return NULL;
  }
  return dev;
}

static void ce_loongarch_timer_remove(struct device_t * dev)
{
  struct clockevent_t * ce = (struct clockevent_t *)dev->priv;
  struct ce_loongarch_timer_pdata_t * pdata = (struct ce_loongarch_timer_pdata_t *)ce->priv;

  if (ce) {
    csr_write(LOONGARCH_CSR_TCFG, 0);
    unregister_clockevent(ce);
    clk_disable(pdata->clk);
    free(pdata->clk);
    free_irq(pdata->irq);
    free_device_name(ce->name);
    free(ce->priv);
    free(ce);
  }
}

static void ce_loongarch_timer_suspend(struct device_t * dev)
{
}

static void ce_loongarch_timer_resume(struct device_t * dev)
{
}

static struct driver_t ce_loongarch_timer = {
  .name   = "ce-loongarch-timer",
  .probe  = ce_loongarch_timer_probe,
  .remove = ce_loongarch_timer_remove,
  .suspend = ce_loongarch_timer_suspend,
  .resume  = ce_loongarch_timer_resume,
};

static __init void ce_loongarch_timer_driver_init(void)
{
  register_driver(&ce_loongarch_timer);
}

static __exit void ce_loongarch_timer_driver_exit(void)
{
  unregister_driver(&ce_loongarch_timer);
}

driver_initcall(ce_loongarch_timer_driver_init);
driver_exitcall(ce_loongarch_timer_driver_exit);
