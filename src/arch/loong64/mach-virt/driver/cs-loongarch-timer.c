/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <clk/clk.h>
#include <clocksource/clocksource.h>

struct cs_loongarch_timer_pdata_t {
	char * clk;
	int cpu;
};

static u64_t cs_loongarch_timer_read(struct clocksource_t * cs)
{
    u64_t val;
    __asm__ __volatile__ (
        "rdtime.d %0, $r0 \n\t"
        : "=r" (val)
    );
    return val;
}

static struct device_t * cs_loongarch_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct cs_loongarch_timer_pdata_t * pdata;

  struct clocksource_t * cs;
  struct device_t * dev;
  char * clk = dt_read_string(n, "clock-name", NULL);

  if(!search_clk(clk))
    return NULL;

  pdata = malloc(sizeof(struct cs_loongarch_timer_pdata_t));
  if (!pdata)
    return NULL;

  cs = malloc(sizeof(struct clocksource_t));
  if(!cs)
  {
    free(pdata);
    return NULL;
  }

  pdata->clk = strdup(clk);
  pdata->cpu = smp_processor_id();

  clk_enable(pdata->clk);
  clocksource_calc_mult_shift(&cs->mult, &cs->shift, clk_get_rate(pdata->clk), 1000000000ULL, 10);
  cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  cs->mask = CLOCKSOURCE_MASK(64);
  cs->read = cs_loongarch_timer_read;
  cs->priv = pdata;


  if(!(dev = register_clocksource(cs, drv)))
  {
    clk_disable(pdata->clk);
    free(pdata->clk);
    free_device_name(cs->name);
    free(cs->priv);
    free(cs);
    return NULL;
  }
  return dev;
}

static void cs_loongarch_timer_remove(struct device_t * dev)
{
  struct clocksource_t * cs = (struct clocksource_t *)dev->priv;
  struct cs_loongarch_timer_pdata_t * pdata = (struct cs_loongarch_timer_pdata_t *)cs->priv;

  if (cs) {
    unregister_clocksource(cs);
    clk_disable(pdata->clk);
    free(pdata->clk);
    free_device_name(cs->name);
    free(cs->priv);
    free(cs);
  }
}

static void cs_loongarch_timer_suspend(struct device_t * dev)
{
}

static void cs_loongarch_timer_resume(struct device_t * dev)
{
}

static struct driver_t cs_loongarch_timer = {
  .name   = "cs-loongarch-timer",
  .probe  = cs_loongarch_timer_probe,
  .remove = cs_loongarch_timer_remove,
  .suspend = cs_loongarch_timer_suspend,
  .resume  = cs_loongarch_timer_resume,
};

static __init void cs_loongarch_timer_driver_init(void)
{
    register_driver(&cs_loongarch_timer);
}

static __exit void cs_loongarch_timer_driver_exit(void)
{
    unregister_driver(&cs_loongarch_timer);
}

driver_initcall(cs_loongarch_timer_driver_init);
driver_exitcall(cs_loongarch_timer_driver_exit);
