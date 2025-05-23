char stack0[4096*8];

void uartinit();
void uartputc_temp(char c);
void uartputs_temp(char* s);


void start()
{
    uartinit();
    uartputs_temp("hello,world!");
    while(1)
        ;
}
