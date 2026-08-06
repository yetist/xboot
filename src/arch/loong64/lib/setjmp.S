#include <linkage.h>

.text
/*
 * int setjmp(jmp_buf env);
 * 输入参数 $a0: 缓冲区 env 数组的基地址
 * 返回值 $a0: 首次进入返回 0
 */
ENTRY(setjmp)
    st.d $ra, $a0, 0x0
    st.d $sp, $a0, 0x8
    st.d $fp, $a0, 0x10
    st.d $s0, $a0, 0x18
    st.d $s1, $a0, 0x20
    st.d $s2, $a0, 0x28
    st.d $s3, $a0, 0x30
    st.d $s4, $a0, 0x38
    st.d $s5, $a0, 0x40
    st.d $s6, $a0, 0x48
    st.d $s7, $a0, 0x50
    st.d $s8, $a0, 0x58

    fst.d  $fs0, $a0, 0x60
    fst.d  $fs1, $a0, 0x68
    fst.d  $fs2, $a0, 0x70
    fst.d  $fs3, $a0, 0x78
    fst.d  $fs4, $a0, 0x80
    fst.d  $fs5, $a0, 0x88
    fst.d  $fs6, $a0, 0x90
    fst.d  $fs7, $a0, 0x98

    move $a0, $zero
    jr   $ra
ENDPROC(setjmp)

/*
 * void longjmp(jmp_buf env, int val);
 * 输入参数 $a0: 缓存区 env 基址
 * 输入参数 $a1: 期望的跳转返回值 (val)
 */
ENTRY(longjmp)
    ld.d $ra, $a0, 0x0
    ld.d $sp, $a0, 0x8
    ld.d $fp, $a0, 0x10
    ld.d $s0, $a0, 0x18
    ld.d $s1, $a0, 0x20
    ld.d $s2, $a0, 0x28
    ld.d $s3, $a0, 0x30
    ld.d $s4, $a0, 0x38
    ld.d $s5, $a0, 0x40
    ld.d $s6, $a0, 0x48
    ld.d $s7, $a0, 0x50
    ld.d $s8, $a0, 0x58

    fld.d $fs0, $sp, 0x60
    fld.d $fs1, $sp, 0x68
    fld.d $fs2, $sp, 0x70
    fld.d $fs3, $sp, 0x78
    fld.d $fs4, $sp, 0x80
    fld.d $fs5, $sp, 0x88
    fld.d $fs6, $sp, 0x90
    fld.d $fs7, $sp, 0x98

    /* 将 $a1 中的传回值转存到返回值寄存器 $a0 */
    move $a0, $a1
    /* 规范：若用户传入的返回值为 0，必须强制将其改为 1 */
    bnez $a0, 1f
    li.w $a0, 1
1:
	  jr   $ra
ENDPROC(longjmp)
