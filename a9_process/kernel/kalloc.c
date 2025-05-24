#include "types.h"
#include "param.h"
#include "riscv.h"
#include "memlayout.h"
#include "defs.h"

extern char end[]; // 在kernel.ld中定义。内核镜像结束后的地址

struct run{
    struct run* next;
};

struct {
    struct run* freelist;
}kmem;

/*
void kinit();
void freerange(void* pa_start,void* pa_end);
void kfree(void* pa);
void* kalloc();
void kmemtest();
*/

// 空闲物理地址初始化
void kinit()
{
    freerange(end,(void*)PHYSTOP);
}

// 针对一个物理地址范围进行释放
void freerange(void* pa_start,void* pa_end)
{
    char* p;
    p = (char*)PGROUNDUP((uint64)pa_start);

    for(; p+PGSIZE<=(char*)pa_end; p+=PGSIZE)
        kfree(p);
}

// 释放物理地址到链表中
void kfree(void* pa)
{
    struct run* r; // 这里run指的是run-time时的内存

    if( ((uint64)pa%PGSIZE)!=0 || (char*)pa<end || (uint64)pa>=PHYSTOP )
        panic("kfree panic\n");

    memset(pa,1,PGSIZE); // 填充垃圾值

    r = (struct run*)pa;

    r->next = kmem.freelist; //第一次为空值

    kmem.freelist = r;
}

// 分配1个页的物理内存
void* kalloc()
{
    struct run* r;
    r = kmem.freelist;

    if(r)
        kmem.freelist = r->next;

    if(r)
        memset((char*)r,5,PGSIZE);

    return (void*)r;
}

void kmemtest()
{
    uint64* p1 = (uint64*)kalloc();
    if(p1 != 0)
    {
        *p1 = 0x123456;
        printf("p1=%p\n",p1); //输出地址
    }

    uint64* p2 = (uint64*)kalloc();
    if(p2 != 0)
    {
        *p2 = 0x11223344;
        printf("p2=%p\n",p2); //输出地址
    }

    // 调整下顺序，看看p3申请到的内存地址
    kfree(p1);
    kfree(p2);

    uint64* p3 = (uint64*)kalloc();
    if(p3 != 0)
    {
        *p3 = 0x111222;
        printf("p3=%p\n",p3); //输出地址
    }

    kfree(p3);
}
