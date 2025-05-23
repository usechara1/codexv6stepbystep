#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "defs.h"

// 内核页表
pagetable_t kernel_pagetable;
extern char etext[]; // 在kernel.ld中定义

// 初始化内核页表
void kvminit()
{
    kernel_pagetable = kvmmake();
}

// 启用内核页表
void kvminithart()
{
    // wait for any previous writes to the page table memory to finish.
    sfence_vma();

    w_satp(MAKE_SATP(kernel_pagetable));

    // flush stale entries from the TLB.
    sfence_vma();
}

// 添加映射.将对应的内存进行映射，xv6使用对等映射，虚拟地址和物理地址一样
pagetable_t kvmmake()
{
    pagetable_t kpgtbl;

    kpgtbl = (pagetable_t) kalloc();
    memset(kpgtbl, 0, PGSIZE);

    // uart registers
    kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

    // PLIC
    kvmmap(kpgtbl,PLIC,PLIC,0x400000,PTE_R|PTE_W);

    // virtio
    kvmmap(kpgtbl,VIRTIO0,VIRTIO0,PGSIZE,PTE_R|PTE_W);

    // map kernel text executable and read-only.
    kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

    // map kernel data and the physical RAM we'll make use of.
    kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

    return kpgtbl;
}

//映射函数
void kvmmap(pagetable_t kpgtbl,uint64 va,uint64 pa,uint64 sz,int perm)
{
    if(mappages(kpgtbl, va, sz, pa, perm) != 0)
        panic("kvmmap");
}

// 具体映射函数。0成功，1失败
int mappages(pagetable_t pagetable,uint64 va,uint64 size,uint64 pa,int perm)
{
    uint64 a, last;
    pte_t *pte;

    if((va % PGSIZE) != 0)
        panic("mappages: va not aligned");

    if((size % PGSIZE) != 0)
        panic("mappages: size not aligned");

    if(size == 0)
        panic("mappages: size");

    a = va;
    last = va + size - PGSIZE;
    for(;;){
        if((pte = walk(pagetable, a, 1)) == 0)
            return -1;
        if(*pte & PTE_V)
            panic("mappages: remap");
        *pte = PA2PTE(pa) | perm | PTE_V;
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
    if(va >= MAXVA)
        panic("walk");

    for(int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[PX(level, va)];
        if(*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
                return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

void _pteprint(pagetable_t pagetable, int level)
{
    for(int i = 0; i < 512; i++) {
        pte_t pte = pagetable[i];

        if (pte & PTE_V) {
            for (int j = 0; j <= level; j++)
                printf(".. ");
            // printf("\b%d: pte %p pa %p\n", i, pte, PTE2PA(pte));
            printf("%d: pte %p pa %p\n", i, pte, PTE2PA(pte));
        }
        if ((pte & PTE_V) && (pte & (PTE_R|PTE_X|PTE_W)) == 0) {
            // this PTE points to a lower-level page table.
            // 继续递归遍历
            uint64 child = PTE2PA(pte);
            _pteprint((pagetable_t)child, level+1);
        }
    }
}

void vmprint(pagetable_t pagetable)
{
    printf("page table %p\n", pagetable);
    _pteprint(pagetable, 0);
}

void vmprint_kernel()
{
    vmprint(kernel_pagetable);
}


