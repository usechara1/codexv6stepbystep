#include "types.h"
#include "riscv.h"
#include "param.h"
#include "memlayout.h"
#include "defs.h"

/*
void plicinit();
void plicinithart();
int plic_claim();
void plic_complete(int irq);
*/

// 配置中断优先级别
void plicinit()
{
    *(uint32*)(PLIC + UART0_IRQ*4) = 1;
    *(uint32*)(PLIC + VIRTIO0_IRQ*4) = 1; //配置磁盘中断优先级
}

// 每个核心都要执行plic初始化
void plicinithart()
{
    int hart = cpuid();

    // 配置s模式响应中断
    // 配置s模式响应中断
    *(uint32*)PLIC_SENABLE(hart) = (1 << UART0_IRQ) | (1 << VIRTIO0_IRQ);

    // 配置s模式下的阈值，阈值位0,表示可以响应任何设备中断
    *(uint32*)PLIC_SPRIORITY(hart) = 0;
}

// 请求plic获得具体的中断源，即哪个设备发生了中断
int plic_claim()
{
    int hart = cpuid();
    int irq = *(uint32*)PLIC_SCLAIM(hart);
    return irq;
}

// 告知PLIC已经处理完成了此中断源的中断，可以接着响应下一个
void plic_complete(int irq)
{
    int hart = cpuid();
    *(uint32*)PLIC_SCLAIM(hart) = irq;
}
