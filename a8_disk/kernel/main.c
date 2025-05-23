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
    //kmemtest();
    uartputs_temp("Step 2\n");

    kvminit();
    //kvminithart();
    //vmprint_kernel();
    uartputs_temp("Step 3\n");

    plicinit();
    plicinithart(); // 每个核心都需要初始化
    uartputs_temp("Step 4\n");

    virtio_disk_init();
    uartputs_temp("Step 5\n");

    // 配置s模式下响应中断、异常
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    virtio_disk_test();
    printf("END x\n");


    while(1)
        ;
}
