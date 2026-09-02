/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2026 Xiaotian Wu <yetist@gmail.com>
 * SPDX-FileContributor: Modified by Xiaotian Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <xboot.h>
#include <loong64.h>
#include <interrupt/interrupt.h>

extern void uart_puts(const char *str);

struct pt_regs_t {
	unsigned long x[32];
	unsigned long estat;
	unsigned long era;
	unsigned long badv;
	unsigned long badi;
};

//#define LOONGARCH_CSR_ECFG    0x4
//#define LOONGARCH_CSR_ESTAT   0x5
//#define LOONGARCH_CSR_ERA     0x6
//#define LOONGARCH_CSR_BADV    0x7
//#define LOONGARCH_CSR_BADI    0x8
//#define LOONGARCH_CSR_EENTRY  0xc
//#define LOONGARCH_CSR_SAVE0   0x30

#define ESTAT_ECOCD_MASK    0x3f0000
#define ESTAT_ESUBCOCD_MASK 0x7fc00000
#define ESTAT_ECODE_SYS     0xb

static const char * ecode_names[] = {
  "It is an INTerrupt",
  "Page Invalid exception for Load operation",
  "Page Invalid exception for Store operation",
  "Page Invalid exception for Fetch operation",
  "Page Modification Exception",
  "Page Non-Readable exception",
  "Page Non-eXecutable exception",
  "Page Privilege level Illegal exception",
  "NULL",
  "Address aLignment fault Exception",
  "Bound Check Exception",
  "SYStem call exception",
  "BReaKpoint exception",
  "Instruction Non-defined Exception",
  "Instruction Privilege error Exception",
  "Floating-Point instruction Disable exception",
  "128-bit vector (SIMD instructions) eXpansion instruction Disable exception",
  "256-bit vector (Advanced SIMD instructions) eXpansion instruction Disable exception",
  "NULL",
  "NULL",
  "Binary Translation expansion instruction Disable exception",
  "Binary Translation related exceptions",
  "Guest Sensitive Privileged Resource exception",
  "HyperVisor Call exception",
  "NULL",
	"Reserved",
};

static void show_regs(struct pt_regs_t * regs)
{
  uint8_t ecode = (regs->estat & ESTAT_ECOCD_MASK) >> 16;
	uint8_t esubcode = (regs->estat & ESTAT_ESUBCOCD_MASK) >> 22;

  switch(ecode)
  {
    case 0x8:
      if (esubcode == 0) {
          LOG("Exception:          %s\r\n", "ADdress error Exception for Fetching instructions");
      } else {
          LOG("Exception:          %s\r\n", "ADdress error Exception for Memory access instructions");
      }
			break;
    case 0x12:
      if (esubcode == 0) {
          LOG("Exception:          %s\r\n", "Floating-Point error Exception");
      } else {
          LOG("Exception:          %s\r\n", "Vecctor Floating-Point error Exception");
      }
			break;
    case 0x13:
      if (esubcode == 0) {
          LOG("Exception:          %s\r\n", "WatchPoint Exception for Fetch watchpoint");
      } else {
          LOG("Exception:          %s\r\n", "WatchPoint Exception for Memory load/store watchpoint");
      }
			break;
    case 0x18:
      if (esubcode == 0) {
          LOG("Exception:          %s\r\n", "Guest CSR Software Change exception");
      } else {
          LOG("Exception:          %s\r\n", "Guest CSR Hardware Change exception");
      }
			break;
		default:
        LOG("Exception:          %s\r\n", ecode_names[ecode]);
			break;
  }
  LOG("Exception Status:   %p\r\n", (void*) regs->estat);
	LOG("Bad instruction pc: %p\r\n", (void*) regs->era);
	LOG("Bad address:        %p\r\n", (void*) regs->badv);
	LOG("Bad instruction:    %p\r\n", (void*) regs->badi);
	LOG("Stored ra:          %p\r\n", (void*) regs->x[1]);
	LOG("Stored sp:          %p\r\n", (void*) regs->x[3]);
}

void loong64_handle_exception(struct pt_regs_t * regs)
{
	//csr_write(LOONGARCH_CSR_SAVE0, (virtual_addr_t)regs);
  show_regs(regs);

  uint64_t era = regs->era;
  era += 4;
	csr_write(LOONGARCH_CSR_ERA, era);
}
