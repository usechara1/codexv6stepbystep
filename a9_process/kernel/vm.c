#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "defs.h"

// 内核页表
pagetable_t kernel_pagetable;
extern char etext[]; // 在kernel.ld中定义

void kvminit();
void kvminithart();
pagetable_t kvmmake();
void kvmmap(pagetable_t kpgtbl,uint64 va,uint64 pa,uint64 sz,int perm);
int mappages(pagetable_t pagetable,uint64 va,uint64 size,uint64 pa,int perm);
pte_t* walk(pagetable_t pagetable,uint64 va,int alloc);

// 初始化内核页表
void kvminit()
{
    kernel_pagetable = kvmmake();
}

// 启用内核页表
void kvminithart()
{
    sfence_vma();
    w_satp(MAKE_SATP(kernel_pagetable));
    sfence_vma();
}

// 添加映射
pagetable_t kvmmake()
{
    pagetable_t kpgtbl;

    kpgtbl = (pagetable_t)kalloc();
    memset(kpgtbl,0,PGSIZE);

    // 映射
    // uart register
    kvmmap(kpgtbl,UART0,UART0,PGSIZE,PTE_R | PTE_W);
    // PLIC
    kvmmap(kpgtbl,PLIC,PLIC,0x400000,PTE_R|PTE_W);
    // virtio
    kvmmap(kpgtbl,VIRTIO0,VIRTIO0,PGSIZE,PTE_R|PTE_W);
    // kernel text
    kvmmap(kpgtbl,KERNBASE,KERNBASE,(uint64)etext-KERNBASE,PTE_R | PTE_X);
    // kernel other
    kvmmap(kpgtbl,(uint64)etext,(uint64)etext,PHYSTOP-(uint64)etext,PTE_R|PTE_W);

    return kpgtbl;
}

// 添加映射
void kvmmap(pagetable_t kpgtbl,uint64 va,uint64 pa,uint64 sz,int perm)
{
    if(mappages(kpgtbl, va, sz, pa, perm) != 0 )
        panic("kvmmap");
}

// 创建PTE 0成功 -1失败
int mappages(pagetable_t pagetable,uint64 va,uint64 size,uint64 pa,int perm)
{
    uint64 a,last;
    pte_t* pte;

    if((va%PGSIZE) != 0)
        panic("mappages: va not aligned");

    if((size%PGSIZE) != 0)
        panic("mappages: size of aligned");

    if( size == 0 )
        panic("mappages: size");

    a = va;
    last = va+size-PGSIZE;

    for(;;)
    {
        if( (pte=walk(pagetable,a,1)) == 0 )
            return -1;
        if(*pte & PTE_V) // 若有效，肯定多次映射，直接返回错误
            panic("mappages: remap");
        *pte=PA2PTE(pa) | perm | PTE_V;
        if(a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

//返回页表中va对应的pte地址，页表项page table entry
//若alloc为0,则不创建页表项，若alloc为1,则创建页表项
/*
rv39
39..64 must be zero
30..38 9bit of level-2 index
21..29 9bit of level-1 index
12..20 9bit of level-0 index
0..11 12bit of byte offset within the page
*/
pte_t* walk(pagetable_t pagetable,uint64 va,int alloc)
{
    if( va >= MAXVA)
        panic("walk");

    for(int level=2; level>0; level--)
    {
        pte_t* pte = &pagetable[PX(level,va)]; // l2中，取1项
        if(*pte & PTE_V) // 有效
        {
            pagetable = (pagetable_t)PTE2PA(*pte); // 获得此项，指向物理地址
        }else{
            if(!alloc || (pagetable=(pte_t*)kalloc())==0)
                return 0;
            memset(pagetable,0,PGSIZE);
            // 若l1不存在，则分配内存空间，并且给l2中的项赋值
            // 新分配的页表，低12位裁掉，补10位标记位
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }

    return &pagetable[PX(0,va)];
}
