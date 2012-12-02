/************************************
Modulo para configurar o display LCD
Tutorial LPC2378-gcc parte 4
*************************************/
#include <arch/nxp/lpc23xx.h>
#include "lcd.h"
void espera_ms(unsigned);
/* Escreve um comando para o LCD */
void LCDcomando(int c)
{
FIO3PIN0 = c;
CLR_RS;
SET_E;
c++; c--;
CLR_E;
espera_ms(2000);
}

/* Escreve um caractere no LCD */
void LCDputchar(char c)
{
FIO3PIN0 = c;
SET_RS;
SET_E;
c++; c--;
CLR_E;
espera_ms(800);
}

/* Configura Timer e LCD */
void LCDinit(void)
{
/* Configura portas I/O do LCD */
FIO3DIR |= 0xff;
LCD_DIR;
CLR_E;
espera_ms(2000);
LCDcomando(0x38);	/* Configura LCD para 2 linhas */
LCDcomando(1);		/* Limpa display */
LCDcomando(0x0c);	/* Apaga cursor */
}

/* Envia uma mensagem no LCD */
void LCDputs(char *txt)
{
while(*txt) LCDputchar(*txt++);
}

void LCDput2dec(int val)
{
	LCDputnum(val / 10);
	LCDputnum(val % 10);
}

void LCDputfloat(float val)
{
	int inteiro = val;
	int fracionario = ((int)(val*100)%100);
	LCDputpartint(inteiro);
	LCDputchar(',');
	LCDput2dec(fracionario);
}

void LCDputpartint(int val)
{
	int aux;
	aux = val/10;
	if (aux < 10){
		LCDputnum((int)aux);
		LCDputnum((int)val%10);
	}
	else{
		LCDputpartint(aux);
		LCDputnum((int)val%10);
	}
}

void LCDputnum(int val)
{
	LCDputchar(val+'0');
}

void LCDp(int pos, char *txt)
{
	LCDcomando(pos);
	LCDputs(txt);
}

