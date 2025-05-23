.section .text

.global _entry
_entry:
    la sp,stack0    # 加载栈地址到寄存器
    li a0,1024*4    # 加载常量到寄存器
    csrr a1,mhartid # 读取硬件线程ID
    addi a1,a1,1    # a1=hartid+1
    mul a0,a0,a1    # a0=(1024*4) * (mhartid+1)
    add sp,sp,a0    # sp=stack0+a0(上面计算的值)
    # 只设置sp寄存器
    call start      # 直接调用start函数

spin:
    j spin
