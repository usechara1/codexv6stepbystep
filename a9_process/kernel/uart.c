#include "memlayout.h"

//#define THR 0

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                 // receive holding register (for input bytes)
#define THR 0                 // transmit holding register (for output bytes)
#define IER 1                 // interrupt enable register
#define IER_RX_ENABLE (1<<0)
#define IER_TX_ENABLE (1<<1)
#define FCR 2                 // FIFO control register
#define FCR_FIFO_ENABLE (1<<0)
#define FCR_FIFO_CLEAR (3<<1) // clear the content of the two FIFOs
#define ISR 2                 // interrupt status register
#define LCR 3                 // line control register
#define LCR_EIGHT_BITS (3<<0)
#define LCR_BAUD_LATCH (1<<7) // special mode to set baud rate
#define LSR 5                 // line status register
#define LSR_RX_READY (1<<0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1<<5)    // THR can accept another character to send

#define Reg(reg) ((volatile unsigned char*)(UART0+reg))
#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg,v) (*(Reg(reg))=(v))

void uartinit();
void uartputc_temp(char c);
void uartputs_temp(char* s);

//串口初始化
void uartinit()
{
    /*
    1：关闭中断 
    2：设置波特率 
    3：设置8位宽度 
    4：清空输入和输出队列
    */

    // disable interrupt
    WriteReg(IER,0);

    // set baud rate 38.4K
    WriteReg(LCR, LCR_BAUD_LATCH);
    WriteReg(0, 0x03);
    WriteReg(1, 0x00);

    // set word length to 8 bit
    WriteReg(LCR, LCR_EIGHT_BITS);

    // reset and enable FIFO
    WriteReg(FCR, FCR_FIFO_CLEAR | FCR_FIFO_ENABLE);

    // todo 目前只启用接受数据中断
    WriteReg(IER, IER_RX_ENABLE);
}

void uartputc_temp(char c)
{
    WriteReg(THR,c);
}

void uartputs_temp(char* s)
{
    while(*s)
    {
        uartputc_temp(*(s++));
    }
}

// 获得串口中断时，RHR寄存器中的值
int uartgetc()
{
    if(ReadReg(LSR) & 0x1)
        return ReadReg(RHR);
    else
        return -1;
}

// 串口接受到数据时的中断处理函数
void uartintr()
{
    while(1)
    {
        int c = uartgetc();
        if(c == -1)
            break;
        uartputc_temp(c);
    }
}

// 睡眠
void uartsleep(int sec)
{
    int interval = 1000000*10*sec; // 秒为单位
    int count = 0;
    while(count < interval)
        count++;
}
