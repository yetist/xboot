/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOONGARCH64_SMP_H__
#define __LOONGARCH64_SMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xconfigs.h>
#include <loong64.h>

#if defined(CONFIG_MAX_SMP_CPUS) && (CONFIG_MAX_SMP_CPUS > 1) && !defined(__SANDBOX__)
static inline int smp_processor_id(void)
{
	return csr_read(LOONGARCH_CSR_CPUID);
}
#else
static inline int smp_processor_id(void)
{
	return 0;
}
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_SMP_H__ */
