/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOONGARCH64_IRQFLAGS_H__
#define __LOONGARCH64_IRQFLAGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CSR_CRMD_IE_BIT		(1 << 2)

#define arch_local_irq_enable() \
	do { \
		unsigned int mask = CSR_CRMD_IE_BIT; \
		unsigned int val = CSR_CRMD_IE_BIT; \
		__asm__ __volatile__( \
			"csrxchg %0, %1, 0x0\n" \
			"dbar 0\n" \
			: "+r"(val) \
			: "r"(mask) \
			: "memory" \
		); \
	} while(0)

#define arch_local_irq_disable() \
	do { \
		unsigned int mask = CSR_CRMD_IE_BIT; \
		unsigned int val = 0; \
		__asm__ __volatile__( \
			"csrxchg %0, %1, 0x0\n" \
			"dbar 0\n" \
			: "+r"(val) \
			: "r"(mask) \
			: "memory" \
		); \
	} while(0)

#define arch_local_save_flags() \
	({ \
		unsigned long flags; \
		__asm__ __volatile__( \
			"csrrd %0, 0x0\n" \
			: "=r"(flags) \
			: \
			: "memory" \
		); \
		flags; \
	})

#define arch_local_irq_save() \
	({ \
		unsigned long flags = arch_local_save_flags(); \
		arch_local_irq_disable(); \
		flags; \
	})

#define arch_local_irq_restore(flags) \
	do { \
		unsigned int mask = CSR_CRMD_IE_BIT; \
		unsigned int val = (unsigned int)((flags) & CSR_CRMD_IE_BIT); \
		__asm__ __volatile__( \
			"csrxchg %0, %1, 0x0\n" \
			"dbar 0\n" \
			: "+r"(val) \
			: "r"(mask) \
			: "memory" \
		); \
	} while(0)

#define arch_local_irq_disabled() \
	(!(arch_local_save_flags() & CSR_CRMD_IE_BIT))

#define local_irq_enable()			arch_local_irq_enable()
#define local_irq_disable()			arch_local_irq_disable()
#define local_save_flags(flags)		do { flags = arch_local_save_flags(); } while (0)
#define local_irq_save(flags)		do { flags = arch_local_irq_save(); } while (0)
#define local_irq_restore(flags)	arch_local_irq_restore(flags)

#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_IRQFLAGS_H__ */
