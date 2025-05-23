char stack0[4096*8];

void uartputc_temp(char c);

void start()
{
    uartputc_temp('h');
    uartputc_temp('e');
    uartputc_temp('l');
    uartputc_temp('l');
    uartputc_temp('o');
    while(1)
        ;
}
