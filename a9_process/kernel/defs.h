#ifndef __DEFS_H__
#define __DEFS_H__

#include "types.h"

struct cpu;
struct spinlock;
struct buf;

// uart.c
void uartinit();
void uartputc_temp(char c);
void uartputs_temp(char* s);
void uartsleep(int sec);

int uartgetc();
void uartintr();

// kernelvec.S
void kernelvec();
void kernelvec1();
void timervec();

// trap.c
void trapinithart();
void kerneltrap();
int devintr();

// kalloc.c
void kinit();
void freerange(void* pa_start,void* pa_end);
void kfree(void* pa);
void* kalloc();
void kmemtest();

// string.c
void* memset(void *dst, int c, uint n);
int memcmp(const void *v1, const void *v2, uint n);
void* memmove(void *dst, const void *src, uint n);
void* memcpy(void *dst, const void *src, uint n);
int strncmp(const char *p, const char *q, uint n);
char* strncpy(char *s, const char *t, int n);
char* safestrcpy(char *s, const char *t, int n);
int strlen(const char *s);

// printf.c
int printf(char *fmt, ...);
void panic(char *s);

// vm.c
void kvminit();
void kvminithart();
pagetable_t kvmmake();
void kvmmap(pagetable_t kpgtbl,uint64 va,uint64 pa,uint64 sz,int perm);
int mappages(pagetable_t pagetable,uint64 va,uint64 size,uint64 pa,int perm);
pte_t* walk(pagetable_t pagetable,uint64 va,int alloc);

// proc.c
int cpuid();
struct cpu* mycpu();

void procinit();
void user1();
void user2();
void userinit1();
void userinit2();

// spinlock.c
void initlock(struct spinlock* lk,char* name);
void acquire(struct spinlock* lk);
void release(struct spinlock* lk);
int holding(struct spinlock* lk);
void push_off();
void pop_off();

// plic.c
void plicinit();
void plicinithart();
int plic_claim();
void plic_complete(int irq);

// virtio_disk.c
void virtio_disk_init();
void virtio_disk_rw(struct buf* b,int write);
void virtio_disk_intr();
void virtio_disk_test();

#endif //__DEFS_H__
