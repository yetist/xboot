/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <clk/clk.h>
#include <rtc/rtc.h>

#define SYS_TOYWRITE0      0x24
#define SYS_TOYWRITE1      0x28
#define SYS_TOYREAD0       0x2C
#define SYS_TOYREAD1       0x30
#define SYS_RTCCTRL        0x40

struct rtc_ls7a_pdata_t {
  virtual_addr_t virt;
  char * clk;
};

static bool_t rtc_ls7a_settime(struct rtc_t * rtc, struct rtc_time_t * time)
{
  uint32_t t;

  struct rtc_ls7a_pdata_t * pdat = (struct rtc_ls7a_pdata_t *)rtc->priv;
  t = (time->second << 4) | (time->minute << 10) | (time->hour << 16);
  t |= (time->day << 21) | (time->month << 26);
  write32(pdat->virt + SYS_TOYWRITE0, t);
  write32(pdat->virt + SYS_TOYWRITE1, time->year - 1900);

  return TRUE;
}

static bool_t rtc_ls7a_gettime(struct rtc_t * rtc, struct rtc_time_t * time)
{
  struct rtc_ls7a_pdata_t * pdat = (struct rtc_ls7a_pdata_t *)rtc->priv;
  uint32_t t, sec1, sec2;

  do {
    t = read32(pdat->virt + SYS_TOYREAD0);
    time->second  = (t >> 4) & 0x3f;
    time->minute  = (t >> 10) & 0x3f;
    time->hour    = (t >> 16) & 0x1f;
    time->day     = (t >> 21) & 0x1f;
    time->month   = (t >> 26) & 0x3f;
    time->year    = 1900 + read32(pdat->virt + SYS_TOYREAD1);

    sec1 = time->second;
    t = read32(pdat->virt + SYS_TOYREAD0);
    sec2 = (t >> 4) & 0x3f;
  } while(sec1 != sec2);

  return TRUE;
}

static struct device_t * rtc_ls7a_probe(struct driver_t * drv, struct dtnode_t * n)
{
  struct rtc_ls7a_pdata_t * pdat;
  struct rtc_t * rtc;
  struct device_t * dev;
  virtual_addr_t virt = phys_to_virt(IO_BASE | dt_read_address(n));
  char * clk = dt_read_string(n, "clock-name", NULL);
  u32_t val;

  if(!search_clk(clk))
    return NULL;

  pdat = malloc(sizeof(struct rtc_ls7a_pdata_t));
  if(!pdat)
    return NULL;

  rtc = malloc(sizeof(struct rtc_t));
  if(!rtc)
  {
    free(pdat);
    return NULL;
  }

  pdat->virt = virt;
  pdat->clk = strdup(clk);

  rtc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
  rtc->settime = rtc_ls7a_settime;
  rtc->gettime = rtc_ls7a_gettime;
  rtc->priv = pdat;

  clk_enable(pdat->clk);
  val = read32(pdat->virt + SYS_RTCCTRL);
  val |= (1 << 13) | (1 << 11) | (1 << 8);
  write32(pdat->virt + SYS_RTCCTRL, val);
  if(!(dev = register_rtc(rtc, drv)))
  {
    clk_disable(pdat->clk);
    free(pdat->clk);
    free_device_name(rtc->name);
    free(rtc->priv);
    free(rtc);
    return NULL;
  }
  return dev;
}

static void rtc_ls7a_remove(struct device_t * dev)
{
  struct rtc_t * rtc = (struct rtc_t *)dev->priv;
  struct rtc_ls7a_pdata_t * pdat = (struct rtc_ls7a_pdata_t *)rtc->priv;

  if(rtc)
  {
    unregister_rtc(rtc);
    clk_disable(pdat->clk);
    free(pdat->clk);
    free_device_name(rtc->name);
    free(rtc->priv);
    free(rtc);
  }
}

static void rtc_ls7a_suspend(struct device_t * dev)
{
}

static void rtc_ls7a_resume(struct device_t * dev)
{
}

static struct driver_t rtc_ls7a = {
  .name		= "rtc-ls7a",
  .probe		= rtc_ls7a_probe,
  .remove		= rtc_ls7a_remove,
  .suspend	= rtc_ls7a_suspend,
  .resume		= rtc_ls7a_resume,
};

static __init void rtc_ls7a_driver_init(void)
{
  register_driver(&rtc_ls7a);
}

static __exit void rtc_ls7a_driver_exit(void)
{
  unregister_driver(&rtc_ls7a);
}

driver_initcall(rtc_ls7a_driver_init);
driver_exitcall(rtc_ls7a_driver_exit);
