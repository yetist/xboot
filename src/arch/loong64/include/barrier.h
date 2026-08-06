/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef __LOONGARCH64_BARRIER_H__
#define __LOONGARCH64_BARRIER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DBAR(hint) __asm__ __volatile__("dbar %0 " : : "I"(hint) : "memory")

#define crwrw   0b00000
#define cr_r_   0b00101
#define c_w_w   0b01010

#define orwrw   0b10000
#define or_r_   0b10101
#define o_w_w   0b11010

#define mb()    DBAR(crwrw)
#define rmb()   DBAR(cr_r_)
#define wmb()   DBAR(c_w_w)

#define smp_mb()  DBAR(orwrw)
#define smp_rmb() DBAR(or_r_)
#define smp_wmb() DBAR(o_w_w)

#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_BARRIER_H__ */
