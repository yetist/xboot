/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <loong64.h>
#include <command/command.h>

#define LOONGARCH_CFG_PRID    0x0

char buf[128];
static char* to_binstr(uint64_t num)
{
    memset(buf, 0, sizeof(buf));
    strcat(buf, "0b");

    // 如果是 0，直接打印并返回
    if (num == 0) {
        strcat(buf, "0");
        return buf;
    }

    // 寻找最高位的 1，跳过前面的前导 0（可选，让输出更紧凑）
    int start_bit = 63;
    while ((start_bit >= 0) && !((num >> start_bit) & 1)) {
        start_bit--;
    }

    // 从最高有效位开始，逐位打印 '0' 或 '1'
    for (int i = start_bit; i >= 0; i--) {
        char* bit = ((num >> i) & 1) ? "1" : "0";
        strcat(buf, bit);
        if ((i % 4 == 0) && (i != 0)) {
            strcat(buf, "_");
        }
    }
  return buf;
}

static void usage(void)
{
	printf("usage:\r\n");
	printf("    cpuinfo\r\n");
}

static int do_cpuinfo(int argc, char ** argv)
{
	uint64_t v;

	printf("Built for LoongArch using GCC %s on %s, at %s.\r\n", __VERSION__, __DATE__, __TIME__);
  v = cpu_cfg(LOONGARCH_CFG_PRID);
	printf("prid:       0x%016lx\r\n", v);
  v = cpu_cfg(0x1);
	printf("cpucfg01:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x2);
	printf("cpucfg02:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x3);
	printf("cpucfg03:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x4);
	printf("cpucfg04:   0x%016lx\r\n", v);
  v = cpu_cfg(0x5);
	printf("cpucfg05:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x6);
	printf("cpucfg06:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x10);
	printf("cpucfg16:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x11);
	printf("cpucfg17:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x12);
	printf("cpucfg18:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x13);
	printf("cpucfg19:   0x%016lx(%s)\r\n", v, to_binstr(v));
  v = cpu_cfg(0x14);
	printf("cpucfg20:   0x%016lx(%s)\r\n", v, to_binstr(v));

	v = csr_read(LOONGARCH_CSR_CRMD);
	printf("crmd:       0x%016lx(%s)\r\n", v, to_binstr(v));
	v = csr_read(LOONGARCH_CSR_EUEN);
	printf("euen:       0x%016lx(%s)\r\n", v, to_binstr(v));
	v = csr_read(LOONGARCH_CSR_CPUID);
	printf("cpuid:      0x%016lx(%s)\r\n", v, to_binstr(v));

	return 0;
}

static struct command_t cmd_cpuinfo = {
	.name	= "cpuinfo",
	.desc	= "show information about CPU",
	.usage	= usage,
	.exec	= do_cpuinfo,
};

static __init void cpuinfo_cmd_init(void)
{
	register_command(&cmd_cpuinfo);
}

static __exit void cpuinfo_cmd_exit(void)
{
	unregister_command(&cmd_cpuinfo);
}

command_initcall(cpuinfo_cmd_init);
command_exitcall(cpuinfo_cmd_exit);
