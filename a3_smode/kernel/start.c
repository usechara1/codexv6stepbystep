#include "types.h"
#include "riscv.h"

char stack0[4096*8];

void main();

void start()
{
    // jump to main

    /*
    1：设置mret返回值的模式为s模式
    2：设置mret的返回地址
    3：关闭地址转换和保护
    4：在s模式下，代理所有中断和异常。
    5：允许s模式访问所有物理内存
    6：时钟中断初始化
    7：执行mret指令
    */
    
    unsigned long x = r_mstatus();
    x &= ~MSTATUS_MPP_MASK; // 清空之前的模式
    x |= MSTATUS_MPP_S;     // 设置之前的模式为s模式
    w_mstatus(x);

    w_satp(0);

    w_mepc((uint64)main);

    w_medeleg(0xffff); //代理所有中断和异常
    w_mideleg(0xffff);
    // external timer software
    w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

    w_pmpaddr0(0x3fffffffffffffull);
    w_pmpcfg0(0xf);

    // todo 时钟中断的初始化

    int id = r_mhartid();
    w_tp(id);

    asm volatile("mret");
}
