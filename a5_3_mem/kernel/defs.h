#ifndef __DEFS_H__
#define __DEFS_H__

#include "types.h"

// uart.c
void uartinit();
void uartputc_temp(char c);
void uartputs_temp(char* s);

// kernelvec.S
void kernelvec();
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

#endif //__DEFS_H__
