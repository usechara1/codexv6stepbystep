#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "proc.h"
#include "spinlock.h"
#include "defs.h"

/*
void initlock(struct spinlock* lk,char* name);
void acquire(struct spinlock* lk);
void release(struct spinlock* lk);
int holding(struct spinlock* lk);
void push_off();
void pop_off();
*/

// 初始化锁
void initlock(struct spinlock* lk,char* name)
{
    lk->locked = 0;

    lk->name = name;
    lk->cpu = 0;
}

// 请求自旋锁
void acquire(struct spinlock* lk)
{
    push_off();

    if(holding(lk))
        panic("acquire");

    while(__sync_lock_test_and_set(&lk->locked,1)!=0)
        ;

    __sync_synchronize();
    lk->cpu = mycpu();
}

// 释放锁
void release(struct spinlock* lk)
{
    if(!holding(lk))
        panic("release");

    lk->cpu = 0;
    __sync_synchronize();
    __sync_lock_release(&lk->locked);
    pop_off();
}

// 是否持有自旋锁
int holding(struct spinlock* lk)
{
    int r;
    r = ( lk->locked && lk->cpu==mycpu());
    return r;
}

//关闭中断，且增加中断嵌套
void push_off()
{
    int old = intr_get();

    intr_off();
    if(mycpu()->noff == 0)
        mycpu()->intena = old;
    mycpu()->noff += 1;
}

// 打开中断，且较少中断潜逃
void pop_off()
{
    struct cpu* c = mycpu();

    if(intr_get())
        panic("pop_off - interruptible");
    if(c->noff < 1)
        panic("pop_off");

    c->noff -= 1;
    if(c->noff==0 && c->intena)
        intr_on();
}
