/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define LOONGARCH_CSR_CRMD    0x0     /* 当前运行模式与全局中断控制寄存器 */
#define LOONGARCH_CSR_PRMD    0x1     /* 异常前状态寄存器 */
#define LOONGARCH_CSR_EUEN    0x2     /* 扩展部件使能寄存器（FPU 硬件使能） */
#define LOONGARCH_CSR_ECFG    0x4     /* 局部中断使能配置寄存器 */
#define LOONGARCH_CSR_ESTAT   0x5     /* 异常与中断状态指示寄存器 */
#define LOONGARCH_CSR_ERA     0x6
#define LOONGARCH_CSR_BADV    0x7
#define LOONGARCH_CSR_BADI    0x8
#define LOONGARCH_CSR_EENTRY  0xc
#define LOONGARCH_CSR_CPUID   0x20    /* 龙芯物理核心号寄存器 (0, 1, 2, 3...) */
#define LOONGARCH_CSR_SAVE0   0x30
#define LOONGARCH_CSR_DMW0    0x180   /* 直接映射窗口 0 控制寄存器 */
#define LOONGARCH_CSR_DMW1    0x181   /* 直接映射窗口 1 控制寄存器 */

#define TO_STR_HELPER(x) #x
#define TO_STR(x) TO_STR_HELPER(x)

#define cpu_cfg(cfg)                            \
({                                              \
  register unsigned long __v;                   \
 __asm__ __volatile__("cpucfg %0, %1"           \
            : "=r"(__v)                         \
            : "r"(cfg)                          \
            : "memory");                        \
  __v;                                          \
})

#define csr_read(csr)                           \
({                                              \
  register unsigned long __v;                   \
 __asm__ __volatile__("csrrd %0, " TO_STR(csr)  \
            : "=r"(__v) :                       \
            : "memory");                        \
  __v;                                          \
})

#define csr_write(csr, val)                           \
    do {                                              \
        uint64_t __tmp = (val);                       \
	      unsigned long __v = (unsigned long)(val);     \
        __asm__ __volatile__("csrwr %0, " TO_STR(csr) \
        ::"r"(__v)                                    \
			  : "memory");                                  \
    } while (0)

#if 0
#define csr_swap(csr, val)							\
({													\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrw %0, " #csr ", %1"	\
			      : "=r" (__v) : "rK" (__v)			\
			      : "memory");						\
	__v;											\
})

#define csr_read_set(csr, val)						\
({													\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrs %0, " #csr ", %1"	\
			      : "=r" (__v) : "rK" (__v)			\
			      : "memory");						\
	__v;											\
})

#define csr_set(csr, val)							\
({													\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrs " #csr ", %0"		\
			      : : "rK" (__v)					\
			      : "memory");						\
})

#define csr_read_clear(csr, val)					\
({													\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrrc %0, " #csr ", %1"	\
			      : "=r" (__v) : "rK" (__v)			\
			      : "memory");						\
	__v;											\
})

#define csr_clear(csr, val)							\
({													\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrc " #csr ", %0"		\
			      : : "rK" (__v)					\
			      : "memory");						\
})
#endif


#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)


#ifdef __cplusplus
}
#endif
