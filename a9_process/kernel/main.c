#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

extern struct process proc[];

void main()
{
    uartinit();
    trapinithart();
    uartputs_temp("Step 1\n");

    kinit();
    kmemtest();
    uartputs_temp("Step 2\n");
    
    kvminit();
    //kvminithart(); // 禁止启用内核分页
    uartputs_temp("Step 3\n");

    plicinit();
    plicinithart(); // 每个核心都需要初始化
    uartputs_temp("Step 4\n");

    virtio_disk_init();
    uartputs_temp("Step 5\n");

    procinit();
    userinit1();
    userinit2();
    uartputs_temp("Step 6\n");

    // 切换到第一个进程中，并进入u模式下
    w_sepc(proc[0].c.pc); //切换到第一个进程中
    asm volatile("mv sp,%0" : : "r"(proc[0].c.sp));
    w_sscratch((uint64)(&proc[0]));

    // 配置s模式下响应中断、异常
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    //virtio_disk_test();
    //printf("END x\n");

    unsigned long x = r_sstatus();
    x &= ~SSTATUS_SPP; // clear spp to 0 for user mode
    x |= SSTATUS_SPIE; // enable interrupts in user mode
    w_sstatus(x);
    asm volatile("sret"); // 返回到U模式下

    while(1)
        ;
}
