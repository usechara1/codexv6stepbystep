
void uartinit();
void uartputc_temp(char c);
void uartputs_temp(char* s);


void main()
{
    uartinit();
    uartputs_temp("hello,world,s mode!");
    while(1)
        ;
}
