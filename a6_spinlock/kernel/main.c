#include "types.h"
#include "riscv.h"
#include "defs.h"

void vmprint_kernel(); 

void main()
{
    uartinit();
    trapinithart();
    uartputs_temp("Step 1\n");

    kinit();
    kmemtest();
    uartputs_temp("Step 2\n");

    kvminit();
    //kvminithart();
    vmprint_kernel();
    uartputs_temp("Step 3\n");

    // 配置s模式下响应中断、异常
    w_sstatus(r_sstatus() | SSTATUS_SIE);

    while(1)
        ;
}
