/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOONGARCH64_SETJMP_H__
#define __LOONGARCH64_SETJMP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 定义标准的 jmp_buf 缓冲区大小。
 * LoongArch64 规范需要保存 $ra, $sp, $fp 以及 $s0~$s8 总共 12 个核心寄存器，预留 13 个 64 位空间。
 */
typedef unsigned long long jmp_buf[21];

/* 前置标准 C 异常跳转接口声明 */
extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif /* __LOONGARCH64_SETJMP_H__ */
