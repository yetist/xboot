/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOONGARCH64_SPINLOCK_H__
#define __LOONGARCH64_SPINLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <irqflags.h>
#include <atomic.h>

#define SPIN_LOCK_UNLOCKED	{ 0 }
#define SPIN_LOCK_INIT()	{ 0 }

static inline void spin_lock_init(spinlock_t * lock)
{
	lock->lock = 0;
}

static inline void spin_lock(spinlock_t * lock)
{
	while (__builtin_expect((lock->lock != 0), 0) ||
	       __builtin_expect((__atomic_test_and_set(&(lock->lock), __ATOMIC_ACQUIRE)), 0)) {
		__asm__ __volatile__("nop" : : : "memory");
	}
}

static inline void spin_unlock(spinlock_t * lock)
{
  __atomic_clear(&(lock->lock), __ATOMIC_RELEASE);
}

static inline int spin_trylock(spinlock_t * lock)
{
	return !__atomic_test_and_set(&(lock->lock), __ATOMIC_ACQUIRE);
}

#define spin_lock_irq(lock) \
	do { arch_local_irq_disable(); spin_lock(lock); } while (0)

#define spin_unlock_irq(lock) \
	do { spin_unlock(lock); arch_local_irq_enable(); } while (0)

#define spin_lock_irqsave(lock, flags) \
	do { flags = arch_local_irq_save(); spin_lock(lock); } while (0)

#define spin_unlock_irqrestore(lock, flags) \
	do { spin_unlock(lock); arch_local_irq_restore(flags); } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_SPINLOCK_H__ */
