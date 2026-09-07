/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#define ACPI_REG_BASE    0x100d0000

static int mach_detect(struct machine_t * mach)
{
  return 1;
}

static void mach_smpinit(struct machine_t * mach)
{
}

static void mach_smpboot(struct machine_t * mach, void (*func)(void))
{
  extern void sys_smp_secondary_boot(void (*func)(void));
  sys_smp_secondary_boot(func);
}

static void mach_shutdown(struct machine_t * mach)
{
  virtual_addr_t virt = phys_to_virt(IO_BASE | ACPI_REG_BASE);

  uint32_t v;

  v = read32(virt + 0xc);
  write32(virt + 0xc, v & 0xffffffff);
  write32(virt + 0x14, (0x1 << 13 | 0b111 << 10));
}

static void mach_reboot(struct machine_t * mach)
{
  virtual_addr_t virt = phys_to_virt(IO_BASE | ACPI_REG_BASE);
  write32(virt + 0x30, 0x1);
}

static void mach_sleep(struct machine_t * mach)
{
}

static void mach_cleanup(struct machine_t * mach)
{
}

static void mach_logger(struct machine_t * mach, const char * buf, int count)
{
  virtual_addr_t virt = 0x1fe001e0;
  int i;

  for(i = 0; i < count; i++)
  {
    while((read8(virt + 0x05) & (0x1 << 5)) == 0);
    write8(virt + 0x00, buf[i]);
  }
}

static const char * mach_uniqueid(struct machine_t * mach)
{
  return NULL;
}

static int mach_keygen(struct machine_t * mach, const char * msg, void * key)
{
  return 0;
}

static int mach_verify(struct machine_t * mach)
{
  return 1;
}

static struct machine_t mach_virt = {
  .name		= "virt",
  .desc		= "QEMU LoongArch64 Virtual Development Board",
  .detect 	= mach_detect,
  .smpinit	= mach_smpinit,
  .smpboot	= mach_smpboot,
  .shutdown	= mach_shutdown,
  .reboot		= mach_reboot,
  .sleep		= mach_sleep,
  .cleanup	= mach_cleanup,
  .logger		= mach_logger,
  .uniqueid	= mach_uniqueid,
  .keygen		= mach_keygen,
  .verify		= mach_verify,
};

static __init void mach_virt_init(void)
{
  register_machine(&mach_virt);
}

static __exit void mach_virt_exit(void)
{
  unregister_machine(&mach_virt);
}

machine_initcall(mach_virt_init);
machine_exitcall(mach_virt_exit);
