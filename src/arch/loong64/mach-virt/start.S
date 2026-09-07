#include <xconfigs.h>
#include <loong64.h>

.macro clear_all_regs
    .irp reg, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        or  $r\reg, $zero, $zero
    .endr
.endm

.global _start
_start:
  b reset

/*
 * The actual reset code
 */
reset:
    li.d    $t0, (0x8000 << 48 | 0x1)     # VSEG[63:60]=0x8000, MAT[5:4]=0, PLV0[0]=1, UC xxxx xxxx xxxx
    csrwr   $t0, LOONGARCH_CSR_DMW0
    li.d    $t0, (0x11)                   # VSEG[63:60]=0x0000, MAT[5:4]=1, PLV0[0]=1, CA, 0x0000 xxxx xxxx xxxx
    csrwr   $t0, LOONGARCH_CSR_DMW1

    # jump_to_virt_addr
    #li.d    $t0, (0x9000 << 48)
    #pcaddi  $t1, 0
    #or      $t0, $t0, $t1
    #jirl    $zero, $t0, 0xc

    /* Enable PG */
    li.w    $t0, 0b10110000   # PLV[1:0]=0, IE[2]=0, PG[4]=1, DATF[6:5]=1, DATM[8:7]=1(一致可缓存CC)
    csrwr   $t0, LOONGARCH_CSR_CRMD
    li.w    $t0, 0b0100       # PPLV[1:0]=0, PIE[2]=1, PWE[3]=0
    csrwr   $t0, LOONGARCH_CSR_PRMD
    li.w    $t0, 0b0001       # FPE[0]=1, SXE[1]=0, ASXE[2]=0, BTE[3]=0
    csrwr   $t0, LOONGARCH_CSR_EUEN

    clear_all_regs

    /* 向CSR_EENTRY寄存器写入高位异常向量总入口地址 */
    la.pcrel $t0, vectors
#    li.d    $t4, (0x9000ULL << 48)
#    or      $t0, $t0, $t4
    csrwr   $t0, LOONGARCH_CSR_EENTRY

    /* 开启当前核心的硬件定时器中断硬路由使能（CSR_ECFG 位 11） */
    li.w    $t0, (1 << 11)
    csrwr   $t0, LOONGARCH_CSR_ECFG

	/* Initialize stacks */
    la.pcrel  $t1, _stack_start
    ld.d      $t1, $t1, 0
	  la.pcrel  $t2, _stack_end
    ld.d      $t2, $t2, 0                 # t1=start, t2=end

	  sub.d     $t0, $t2, $t1               # t0=stack_size
    csrrd     $t3, LOONGARCH_CSR_CPUID    # t3=current_cpu
	  li.w      $t4, CONFIG_MAX_SMP_CPUS    # t4=max_cpu
	  div.d     $t0, $t0, $t4               # t0=per_cpu_stack=stack_size/max_cpu

	  mul.d     $t0, $t0, $t3               # t0=t0*t3=per_cpu_stack*current_cpu
	  sub.d     $sp, $t2, $t0               # sp=stack_end-current_start

	/* Initial system uart */
	  bl sys_uart_init

	/* Check processor id, and startup slave cores */
    csrrd     $a0, LOONGARCH_CSR_CPUID
    andi      $a0, $a0, 0xff
    beqz      $a0, 2f
1:
    bl sys_smp_secondary_startup
    b 1b
2:
    nop

	/* Copyself to link address */
	  la.pcrel  $t0, _start
	  la.pcrel  $t1, _image_start
	  ld.d      $t1, $t1, 0
	  beq       $t0, $t1, 1f

	  la.pcrel  $a0, _image_start
	  ld.d      $a0, $a0, 0
	  la.pcrel  $a2, _image_end
	  ld.d      $a2, $a2, 0
	  sub.d     $a2, $a2, $a0
	  la.pcrel  $a1, _start
	  bl        memcpy    # memcpy(_image_start, _start, size)
1: nop

	/* Clear bss section */
	la    $a0, _bss_start
	ld.d  $a0, $a0, 0
	la    $a2, _bss_end
	ld.d  $a2, $a2, 0
	sub.d $a2, $a2, $a0
	li.w  $a1, 0
	bl    memset

	/* Call _main */
	la    $t1, _image_start
	ld.d  $t1, $t1, 0         # t1=_image_start
	la    $t2, _start         # t2=_start
	sub.d $t0, $t1, $t2       # t0=size = _image_start - _start
	la    $a0, _main
	add.d $a0, $a0, $t0       # a0= _main + size
	jr    $a0

_main:
	bl xboot_main
  b _main

/*
 * Exception vectors.
 */
	.align 4
	.globl vectors
vectors:
  csrwr   $sp, LOONGARCH_CSR_SAVE0    # save orig sp into SAVE0
  csrrd   $sp, LOONGARCH_CSR_SAVE0    # let sp is orig sp value
  addi.d  $sp,   $sp, -8 * 36

  st.d    $zero, $sp, 0 * 8
  st.d    $ra,   $sp, 1 * 8
  st.d    $tp,   $sp, 2 * 8
  st.d    $a0,   $sp, 4 * 8
  st.d    $a1,   $sp, 5 * 8
  st.d    $a2,   $sp, 6 * 8
  st.d    $a3,   $sp, 7 * 8
  st.d    $a4,   $sp, 8 * 8
  st.d    $a5,   $sp, 9 * 8
  st.d    $a6,   $sp, 10 * 8
  st.d    $a7,   $sp, 11 * 8
  st.d    $t0,   $sp, 12 * 8
  st.d    $t1,   $sp, 13 * 8
  st.d    $t2,   $sp, 14 * 8
  st.d    $t3,   $sp, 15 * 8
  st.d    $t4,   $sp, 16 * 8
  st.d    $t5,   $sp, 17 * 8
  st.d    $t6,   $sp, 18 * 8
  st.d    $t7,   $sp, 19 * 8
  st.d    $t8,   $sp, 20 * 8
  st.d    $r21,  $sp, 21 * 8
  st.d    $fp,   $sp, 22 * 8
  st.d    $s0,   $sp, 23 * 8
  st.d    $s1,   $sp, 24 * 8
  st.d    $s2,   $sp, 25 * 8
  st.d    $s3,   $sp, 26 * 8
  st.d    $s4,   $sp, 27 * 8
  st.d    $s5,   $sp, 28 * 8
  st.d    $s6,   $sp, 29 * 8
  st.d    $s7,   $sp, 30 * 8
  st.d    $s8,   $sp, 31 * 8

  csrrd    $t0, LOONGARCH_CSR_SAVE0     # t0 is the orig sp value
  #csrwr    $zero, LoongArch_CSR_SAVE0  # 将零 ($zero) 写入 CSR
  csrrd    $s0, LOONGARCH_CSR_ESTAT     #
  csrrd    $t1, LOONGARCH_CSR_ERA       #
  csrrd    $t2, LOONGARCH_CSR_BADV      #
  csrrd    $t3, LOONGARCH_CSR_BADI      #

  st.d    $t0,   $sp, 3 * 8             # Save orig sp
  st.d    $s0,   $sp, 32 * 8            #
  st.d    $t1,   $sp, 33 * 8            #
  st.d    $t2,   $sp, 34 * 8            #
  st.d    $t3,   $sp, 35 * 8            #

  addi.d  $a0,   $sp, 36 * 8            # a0 = orig sp
  csrwr   $a0,   LOONGARCH_CSR_SAVE0    # save orig sp into SAVE0

  move    $a0,   $sp
  bl      loong64_handle_exception

  csrrd   $sp, LOONGARCH_CSR_SAVE0      # get orig sp from SAVE0
  addi.d  $sp, $sp, -8 * 36
  ld.d    $ra, $sp, 1 * 8
  ld.d    $tp, $sp, 2 * 8
  ld.d    $a0, $sp, 4 * 8
  ld.d    $a1, $sp, 5 * 8
  ld.d    $a2, $sp, 6 * 8
  ld.d    $a3, $sp, 7 * 8
  ld.d    $a4, $sp, 8 * 8
  ld.d    $a5, $sp, 9 * 8
  ld.d    $a6, $sp, 10 * 8
  ld.d    $a7, $sp, 11 * 8
  ld.d    $t0, $sp, 12 * 8
  ld.d    $t1, $sp, 13 * 8
  ld.d    $t2, $sp, 14 * 8
  ld.d    $t3, $sp, 15 * 8
  ld.d    $t4, $sp, 16 * 8
  ld.d    $t5, $sp, 17 * 8
  ld.d    $t6, $sp, 18 * 8
  ld.d    $t7, $sp, 19 * 8
  ld.d    $t8, $sp, 20 * 8
  ld.d    $r21,$sp, 21 * 8
  ld.d    $fp, $sp, 22 * 8
  ld.d    $s0, $sp, 23 * 8
  ld.d    $s1, $sp, 24 * 8
  ld.d    $s2, $sp, 25 * 8
  ld.d    $s3, $sp, 26 * 8
  ld.d    $s4, $sp, 27 * 8
  ld.d    $s5, $sp, 28 * 8
  ld.d    $s6, $sp, 29 * 8
  ld.d    $s7, $sp, 30 * 8
  ld.d    $s8, $sp, 31 * 8
  ld.d    $sp, $sp, 3 * 8

  # Return from exception.
  ertn

.align 3
_image_start:
	.dword __image_start
_image_end:
	.dword __image_end
_data_start:
	.dword __data_start
_data_end:
	.dword __data_end
_bss_start:
	.dword __bss_start
_bss_end:
	.dword __bss_end
_stack_start:
	.dword __stack_start
_stack_end:
	.dword __stack_end
