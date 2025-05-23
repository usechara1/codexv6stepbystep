#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "proc.h"
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

    if(which_dev == 1)
    {
        uartputs_temp("external interrupt...\n");
        //#define PLIC_PENDING (PLIC + 0x1000)
        //int t1 = *(uint32*)PLIC_PENDING;

        int irq = plic_claim();
        printf("external interrupt irq=%d\n",irq);
        //printf("external interrupt t1=%d\n",t1);
        //int t2 = *(uint32*)PLIC_PENDING;
        //printf("external interrupt t2=%d\n",t2);
        if(irq == UART0_IRQ)
        {
            printf("this is uart handler...\n");
            uartintr();
            printf("\n\n");
        }

        if(irq == VIRTIO0_IRQ)
        {
            printf("this is virtio handler...\n");
            virtio_disk_intr();
            printf("\n\n");
        }
        if(irq)
            plic_complete(irq);
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

    // 1... 9 64位 supervisor external
    if(scause == 0x8000000000000009L)
    {
        return 1;
    }
    return 0;
}

