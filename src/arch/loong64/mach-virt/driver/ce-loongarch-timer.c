/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <xboot.h>

struct timer_loongarch_pdata_t {
	u64_t period;
};

/*
 * 100% 官方正统规范：最新主线仅保留这一个唯一的、用于动态刷新下一个调度节拍的函数指针成员
 */
static bool_t timer_loongarch_next(struct clockevent_t * ce, u64_t evt)
{
	u64_t tcfg = (evt << 2) | 0x1;
	__asm__ __volatile__(
		"csrwr %0, 0x10\n"
		:
		: "r"(tcfg)
		: "memory"
	);
	return TRUE;
}

static struct device_t * timer_loongarch_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct timer_loongarch_pdata_t * pdat;
	struct clockevent_t * ce;
	struct device_t * dev;

	pdat = malloc(sizeof(struct timer_loongarch_pdata_t));
	if (!pdat)
		return NULL;

	/* QEMU 恒定计数器 100MHz 频率下，10ms 对应的 ticks 值为 1,000,000 */
	pdat->period = 1000000;

	ce = malloc(sizeof(struct clockevent_t));
	if (!ce) {
		free(pdat);
		return NULL;
	}

	ce->name = strdup(n->name);
	ce->mult = 1;
	ce->shift = 0;
	ce->min_delta_ns = 1000;
	ce->max_delta_ns = 1000000000ULL;

	/* 核心对齐：只灌入绝对存在的 .next 成员函数，彻底粉碎任何关于 set 函数的编译错误 */
	ce->next = timer_loongarch_next;
	ce->priv = pdat;

	if (!register_clockevent(ce, drv)) {
		free(ce->name);
		free(ce);
		free(pdat);
		return NULL;
	}

	dev = malloc(sizeof(struct device_t));
	if (!dev) {
		unregister_clockevent(ce);
		free(ce->name);
		free(ce);
		free(pdat);
		return NULL;
	}

	dev->name = strdup(n->name);
	dev->driver = drv;
	dev->priv = ce;

	/*
	 * 终极安全起航：在时钟对象已被成功注册到内核系统并 probe 成功的这最后一个安全一秒，
	 * 我们在此处动态启动初始时钟嘀嗒，并瞬间轰开 CPU 的全局硬件中断使能控制位 IE！
	 */
	timer_loongarch_next(ce, pdat->period);

	__asm__ __volatile__(
		"li.w $t0, 1 << 2\n"      /* CSR_CRMD 的 IE 位为 Bit 2 */
		"csrxchg $t0, $t0, 0x0\n" /* 0x0 即为 CSR_CRMD 寄存器 */
		"dbar 0\n"
		:
		:
		: "t0", "memory"
	);

	return dev;
}

static void timer_loongarch_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;
	struct timer_loongarch_pdata_t * pdat = (struct timer_loongarch_pdata_t *)ce->priv;

	if (ce) {
		unregister_clockevent(ce);
		free(ce->name);
		free(ce);
	}
	if (pdat) {
		free(pdat);
	}
	free(dev->name);
	free(dev);
}

static void timer_loongarch_suspend(struct device_t * dev)
{
}

static void timer_loongarch_resume(struct device_t * dev)
{
}

static struct driver_t timer_loongarch = {
	.name	= "timer-loongarch",
	.probe	= timer_loongarch_probe,
	.remove	= timer_loongarch_remove,
	.suspend = timer_loongarch_suspend,
	.resume  = timer_loongarch_resume,
};

static __init void timer_loongarch_driver_init(void)
{
	register_driver(&timer_loongarch);
}

static __exit void timer_loongarch_driver_exit(void)
{
	unregister_driver(&timer_loongarch);
}

driver_initcall(timer_loongarch_driver_init);
driver_exitcall(timer_loongarch_driver_exit);

void mach_timer_init(void)
{
}
