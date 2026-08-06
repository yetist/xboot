/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>

struct smp_boot_entry_t {
	void (*func)(void);
	atomic_t atomic;
};
struct smp_boot_entry_t __smp_boot_entry[CONFIG_MAX_SMP_CPUS];

void sys_smp_secondary_startup(int cpu)
{
	struct smp_boot_entry_t * e = &__smp_boot_entry[0];

	if(cpu < 0 || cpu >= CONFIG_MAX_SMP_CPUS)
		return;

	e[cpu].func = NULL;
	atomic_set(&e[cpu].atomic, 0);

	while(1)
	{
		while(atomic_cmpxchg(&e[cpu].atomic, 1, 0) != 1)
		{
		}
		if(e[cpu].func)
			e[cpu].func();
	}
}

void sys_smp_secondary_boot(void (*func)(void))
{
	struct smp_boot_entry_t * e = &__smp_boot_entry[0];
	int i;

	for(i = 0; i < CONFIG_MAX_SMP_CPUS; i++)
	{
		e[i].func = func;
		atomic_cmpxchg(&e[i].atomic, 0, 1);
	}
}
