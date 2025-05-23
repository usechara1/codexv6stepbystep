#define UART0 0x10000000L

#define THR 0
#define Reg(reg) ((volatile unsigned char*)(UART0+reg))
#define WriteReg(reg,v) (*(Reg(reg))=(v))

void uartputc_temp(char c)
{
    WriteReg(THR,c);
}
