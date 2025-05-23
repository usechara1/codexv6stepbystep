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

#endif //__DEFS_H__
