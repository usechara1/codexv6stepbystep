#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

struct spinlock{
    uint locked;

    // 调试使用
    char* name;
    struct cpu* cpu;
};

#endif //__SPINLOCK_H__
