/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>

/*
 * 核心挂起循环：
 * 当锁机制或调度器让当前 CPU 核心进入空闲死循环时调用。
 * 使用 idle 0 指令让 CPU 停止流水线，直到被下一次硬件中断唤醒。
 */
void boot_cpu_loop(void)
{
	while(1) {
		__asm__ __volatile__("idle 0" : : : "memory");
	}
}

/* 对齐 XBOOT 核心头文件 include/time/delay.h 中的 u32_t 规约 */
void __attribute__((weak)) udelay(u32_t us)
{
	volatile u32_t loops = us * 100;
	while(loops--);
}

void __attribute__((weak)) mdelay(u32_t ms)
{
	udelay(ms * 1000);
}
