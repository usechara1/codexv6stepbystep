#include "types.h"
#include "riscv.h"
#include "param.h"
#include "proc.h"
#include "defs.h"

struct cpu cpus[NCPU];

/*
int cpuid();
struct cpu* mycpu();
*/

int cpuid()
{
    int id = r_tp(); //riscv未提供直接读取当前cpu编号的函数
    return id;
}

struct cpu* mycpu()
{
    int id = cpuid();
    struct cpu* c = &cpus[id];
    return c;
}

struct process proc[NPROC];

uchar stack1[PGSIZE];
uchar stack2[PGSIZE];
int nextpid = 1;

/*
void procinit();
void user1();
void user2();
void userinit1();
void userinit2();
*/

// 初始化全局进程表
void procinit()
{
    struct process* p;
    for(p=proc; p<&proc[NPROC]; p++)
    {
        p->pid = nextpid;
        nextpid++;
    }
}

// 第一个进程的执行体
void user1()
{
    while(1)
    {
        uartsleep(2);
        printf("a  ");
    }
}

// 第二个进程的执行体
void user2()
{
    while(1)
    {
        uartsleep(2);
        printf("b  ");
    }
}

// 初始化第一个进行
void userinit1()
{
    proc[0].c.sp = (uint64)&(stack1[PGSIZE-1]);
    proc[0].c.pc = (uint64)&(user1);
}

// 初始化第二个进行
void userinit2()
{
    proc[1].c.sp = (uint64)&(stack2[PGSIZE-1]);
    proc[1].c.pc = (uint64)&(user2);
}
