#include "types.h"
#include "param.h"
#include "riscv.h"
#include "memlayout.h"
#include "defs.h"

extern char end[]; // kernel.ld中定义，内核镜像的结束地址

struct run{
    struct run* next;
};

struct{
    struct run* freelist;
}kmem;

// 空闲内存地址的初始化
void kinit()
{
    freerange(end, (void*)PHYSTOP);
}

// 针对一个物理内存的地址范围进行初始化
void freerange(void* pa_start,void* pa_end)
{
    char *p;
    p = (char*)PGROUNDUP((uint64)pa_start);
    for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
        kfree(p);

}

// 将内存地址放置到链表中
void kfree(void* pa)
{
    struct run *r;

    if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    r->next = kmem.freelist; //第一次为0
    kmem.freelist = r;
}

// 分配物理内存地址
void* kalloc()
{
    struct run *r;

    r = kmem.freelist;
    if(r)
        kmem.freelist = r->next;

    if(r)
        memset((char*)r, 5, PGSIZE); // fill with junk
    return (void*)r;
}

void kmemtest()
{
    uint64* p1 = (uint64*)kalloc();
    if(p1 !=0 )
    {
        *p1 = 0x123456;
        printf("p1=%p\n",p1);
    }
    uint64* p2 = (uint64*)kalloc();
    if(p2 !=0 )
    {
        *p2 = 0x11223344;
        printf("p2=%p\n",p2);
    }

    // 释放顺序
    kfree(p1);
    kfree(p2);
    uint64* p3 = (uint64*)kalloc();

    if(p3 !=0 )
    {
        *p3 = 0x11223344;
        printf("p3=%p\n",p3);
    }
}
