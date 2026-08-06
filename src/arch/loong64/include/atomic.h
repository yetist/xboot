/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOONGARCH64_ATOMIC_H__
#define __LOONGARCH64_ATOMIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <barrier.h>

#define ATOMIC_INIT(i)	{ (i) }

static inline int atomic_get(const atomic_t *v)
{
	return *(volatile int *)&(v->counter);
}

static inline int atomic_read(const atomic_t *v)
{
	return *(volatile int *)&(v->counter);
}

static inline void atomic_set(atomic_t *v, int i)
{
	v->counter = i;
}

static inline void atomic_add(atomic_t *v, int i)
{
	int val;
	__asm__ __volatile__(
		"amadd.w %0, %1, %2\n"
		: "=&r"(val)
		: "r"(i), "m"(v->counter)
		: "memory"
	);
}

static inline void atomic_sub(atomic_t *v, int i)
{
	atomic_add(v, -i);
}

static inline int atomic_add_return(atomic_t *v, int i)
{
	int val;
	__asm__ __volatile__(
		"amadd.w %0, %1, %2\n"
		: "=&r"(val)
		: "r"(i), "m"(v->counter)
		: "memory"
	);
	return val + i;
}

static inline int atomic_sub_return(atomic_t *v, int i)
{
	return atomic_add_return(v, -i);
}

static inline void atomic_inc(atomic_t *v)
{
	atomic_add(v, 1);
}

static inline void atomic_dec(atomic_t *v)
{
	atomic_add(v, -1);
}

static inline int atomic_inc_return(atomic_t *v)
{
	return atomic_add_return(v, 1);
}

static inline int atomic_dec_return(atomic_t *v)
{
	return atomic_add_return(v, -1);
}

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	int ret;
	__asm__ __volatile__(
		"1: ll.w %0, %1\n"
		"   bne %0, %2, 2f\n"
		"   move $t0, %3\n"
		"   sc.w $t0, %1\n"
		"   beqz $t0, 1b\n"
		"2:\n"
		: "=&r"(ret), "+m"(v->counter)
		: "r"(old), "r"(new)
		: "t0", "memory"
	);
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_ATOMIC_H__ */
