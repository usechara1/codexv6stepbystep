#include "types.h"
#include "riscv.h"

char stack0[4096*8];

void timerinit();
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

    // 时钟中断的初始化
    timerinit();

    int id = r_mhartid();
    w_tp(id);

    asm volatile("mret");
}

void timerinit()
{
    /*
    1: 打开s模式下的时钟中断
    2: 打开stimecmp扩展功能，为了直接在s模式下操作时钟相关寄存器。
    (备注: 以前只能在m模式下操作)
    3: 允许s模式直接使用stimecmp和时钟
    4: 配置时钟中断的间隔时间
    （注意，在新版本中检测时钟中断也需要变，查看scause的第5位）
    */
    // enable supervisor-mode timer interrupts.
    w_mie(r_mie() | MIE_STIE);

    // enable the sstc extension (i.e. stimecmp).
    w_menvcfg(r_menvcfg() | (1L << 63)); 

    // allow supervisor to use stimecmp and time.
    w_mcounteren(r_mcounteren() | 2);

    // ask for the very first timer interrupt.
    //w_stimecmp(r_time() + 1000000);
    w_stimecmp(r_time() + 1000000*10*2); // 2s
}
