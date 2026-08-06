/*
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  | 0x0 | 0x4 | 0x8 | 0xc | 0x10| 0x14| 0x18| 0x1c|  *
 *  -------------------------------------------------  *
 *  |    fs0    |    fs1    |    fs2    |    fs3    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  | 0x20| 0x24| 0x28| 0x2c| 0x30| 0x34| 0x38| 0x3c|  *
 *  -------------------------------------------------  *
 *  |    fs4    |    fs5    |    fs6    |    fs7    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  *
 *  -------------------------------------------------  *
 *  | 0x40| 0x44| 0x48| 0x4c| 0x50| 0x54| 0x58| 0x5c|  *
 *  -------------------------------------------------  *
 *  |    s0     |    s1     |    s2     |    s3     |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31 |  *
 *  -------------------------------------------------  *
 *  | 0x60| 0x64| 0x68| 0x6c| 0x70| 0x74| 0x78| 0x7c|  *
 *  -------------------------------------------------  *
 *  |    s4     |    s5     |    s6     |    s7     |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  32 |  33 |  34 |  35 |  36 |  37 |  38 |  39 |  *
 *  -------------------------------------------------  *
 *  | 0x80| 0x84| 0x88| 0x8c| 0x90| 0x94| 0x98| 0x9c|  *
 *  -------------------------------------------------  *
 *  |    s8     |    ra     |    pc     |   align   |  *
 *  -------------------------------------------------  *
 */

/*
 * extern void * make_fcontext(void * stack, size_t size, void (*func)(struct transfer_t));
 * $a0: 栈内存基指针 (stack)
 * $a1: 栈空间大小 (size)
 * $a2: 协程入口函数指针 (func)
 * 返回值 $a0: 构建完成的新上下文指针位置
 */
.text
.global make_fcontext
.align 3
.type make_fcontext, %function
make_fcontext:
    /* 将传入的栈地址 a0 向下对齐到 16 字节边界 */
    bstrins.d $a0, $zero, 3, 0

    /* 预留上下文数据空间 0xa0 */
    addi.d    $a0, $a0, -0xa0

    /* a2 为第 3 个参数：协程入口函数地址。存入原本 pc 的位置 (0x90) */
    st.d      $a2, $a0, 0x90

    /* 将 finish 标签地址存入原本 ra 的位置 (0x88)，作为协程退出时的返回地址 */
    la.pcrel  $a4, finish
    st.d      $a4, $a0, 0x88

    /* 返回指向上下文数据的指针 a0 */
    jirl      $r0, $ra, 0

finish:
    /* 退出码为 0 */
    move      $a0, $zero
    /* 调用 exit 退出应用 */
    bl        exit

/*
 * extern struct transfer_t jump_fcontext(void * fctx, void * priv);
 * $a0: 目标协程上下文指针 (fctx)
 * $a1: 传递给目标的私有数据指针 (priv)
 * 返回值通过两个寄存器：$a0 携带旧的 fctx， $a1 携带 priv
 */
.global jump_fcontext
.align 3
.type jump_fcontext, %function
jump_fcontext:
    addi.d  $sp, $sp, -0xa0
	  /* save fs0 - fs7 */
    fst.d   $fs0, $sp, 0x00
    fst.d   $fs1, $sp, 0x08
    fst.d   $fs2, $sp, 0x10
    fst.d   $fs3, $sp, 0x18
    fst.d   $fs4, $sp, 0x20
    fst.d   $fs5, $sp, 0x28
    fst.d   $fs6, $sp, 0x30
    fst.d   $fs7, $sp, 0x38

	  /* save s0 - s8, ra */
    st.d    $s0, $sp, 0x40
    st.d    $s1, $sp, 0x48
    st.d    $s2, $sp, 0x50
    st.d    $s3, $sp, 0x58
    st.d    $s4, $sp, 0x60
    st.d    $s5, $sp, 0x68
    st.d    $s6, $sp, 0x70
    st.d    $s7, $sp, 0x78
    st.d    $s8, $sp, 0x80
    st.d    $ra, $sp, 0x88

	  /* save ra as pc */
    st.d    $ra, $sp, 0x90

	/* store sp (pointing to context-data) in a2 */
    move    $a2, $sp

	/* restore sp (pointing to context-data) from a0 */
    move    $sp, $a0

    /* load fs0 - fs7 */
    fld.d   $fs0, $sp, 0x00
    fld.d   $fs1, $sp, 0x08
    fld.d   $fs2, $sp, 0x10
    fld.d   $fs3, $sp, 0x18
    fld.d   $fs4, $sp, 0x20
    fld.d   $fs5, $sp, 0x28
    fld.d   $fs6, $sp, 0x30
    fld.d   $fs7, $sp, 0x38

    /* load s0 - s8, ra */
    ld.d    $s0, $sp, 0x40
    ld.d    $s1, $sp, 0x48
    ld.d    $s2, $sp, 0x50
    ld.d    $s3, $sp, 0x58
    ld.d    $s4, $sp, 0x60
    ld.d    $s5, $sp, 0x68
    ld.d    $s6, $sp, 0x70
    ld.d    $s7, $sp, 0x78
    ld.d    $s8, $sp, 0x80
    ld.d    $ra, $sp, 0x88

    /* return transfer_t from jump */
    /* pass transfer_t as first arg in context function */
    move    $a0, $a2

    /* load pc */
    ld.d    $a2, $sp, 0x90 #pc

    /* restore stack */
    addi.d  $sp, $sp, 0xa0
    jr      $a2
