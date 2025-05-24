#ifndef __PROC_H__
#define __PROC_H__

struct cpu{
    int noff; //嵌套深度
    int intena; //中断关闭前的中断状态 interrup enable?
};

struct context{
    //保存所有寄存器的状态
    uint64 ra;
    uint64 sp;
    uint64 gp;
    uint64 tp;
    uint64 t0;
    uint64 t1;
    uint64 t2;
    uint64 s0;
    uint64 s1;
    uint64 a0;
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 a4;
    uint64 a5;
    uint64 a6;
    uint64 a7;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
    uint64 t3;
    uint64 t4;
    uint64 t5;
    uint64 t6;

    uint64 pc; // offset: 31 *8 =248
};

struct process{
    struct context c;
    int pid;
};
#endif //__PROC_H__
