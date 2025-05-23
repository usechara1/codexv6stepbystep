#include "types.h"
#include "riscv.h"
#include "defs.h"

/*
void trapinithart();
void kerneltrap();
int devintr();
*/

// 设置stvec寄存器
void trapinithart()
{
    w_stvec((uint64)kernelvec);
}

// s模式下中断入口
void kerneltrap()
{
    int which_dev = 0;

    which_dev = devintr();
    if(which_dev == 0)
    {
        uartputs_temp("panic: interrupt not handle\n");
        while(1)
            ;
    }
    if(which_dev == 2)
    {
        uartputs_temp("timer interrupt...\n");
    }
}

// 获得中断、异常原因
int devintr()
{
    uint64 scause = r_scause();

    // 老版本
    // 1 ... 1 64位 supervior software interrupt
    //if(scause == 0x8000000000000001L)

    // 新版本
    // 1.... 101 就是5 Supervisor timer interrupt
    //           0x8000000000000005L
    if(scause == 0x8000000000000005L)
    {
        // ask for the next timer interrupt. this also clears
        // the interrupt request. 1000000 is about a tenth
        // of a second.
        w_stimecmp(r_time() + 1000000 * 10 * 2);  // 2s
        return 2;
    }

    return 0;
}

