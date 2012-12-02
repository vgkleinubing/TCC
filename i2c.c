/************************************
Modulo generico para usar o I2C 
Marcos A. Stemmer
*************************************/
#include "LPC2300.h"
#include "i2c.h"
void xputc(int c);

/* Inicializa I2C */
void ini_i2c(void)
{
#ifdef USA_I2C2
PCONP |=0x04000000;	/* Liga energia do I2C2 */
PINSEL0 &= 0xff0fffff;
PINSEL0 |= 0x00a00000;	// Seleciona SDA2 e SCL2 no P0.10 e P0.11
#else
PCONP |=0x00000080;     /* Liga energia do I2C0 */
PINSEL1 |= 0x01400000;	// Selciona os pinos SDA0 e SCL0
#endif
I2cCONCLR = 0xff;	// Limpa o I2cCON
I2cCONSET = 0x40;	/* Habilita o I2C-2 um modo mestre */
I2cSCLH   = 100;	/* Tempo alto do SCL	Clock de 60kHz */
I2cSCLL   = 100;	/* Tempo baixo do SCL	*/
}

/*
addri2c	Endereco do dispositivo
buffer	Onde estao os dados
n	Numero de bytes que serao escritos 
Retorna 0=sucesso -1=fracasso */
#define TOINI 1000
int escreve_i2c(int addri2c, char *buffer, int n)
{
int i=0;	// Indice para acessar o buffer
int to;
I2cCONCLR = 0x1f;
I2cCONSET = 0x60;	// Inicia gerando o START em modo mestre
to = TOINI;
do	{
	switch(I2cSTAT){
		case 0x08:	/* Terminou o START BIT */
			I2cCONCLR=0x28;		// Limpa o START e SI
			I2cDAT = addri2c;	// Envia endereco
			to = TOINI;
			break;
		case 0x20:	// Endereco com NACK
			I2cCONSET=0x10;	// Envia stop
			I2cCONCLR = 8;	// Limpa SI
			return -1;	// Indica erro de leitura
		case 0x18:	// Enviou endereco e recebeu ACK
		case 0x28:	// Enviou dado recebendo ACK
			if(i < n) I2cDAT = buffer[i++];	// Se tem dados envia
				else { I2cCONSET=0x10; i=-1; }	// se nao tem, envia stop
			I2cCONCLR = 8;	// Limpa SI
			to = TOINI;
			break;
		case 0x48:	// Endereco de leitura com NACK
			I2cCONSET=0x10;	// Envia stop
			i=-1;
			I2cCONCLR=8;
			return -1;	// Indica erro de leitura
		case 0x40:	// Enderco de leitura com ACK
			I2cCONSET = 0x04;	// Enviar ACK no recebimento de dado
			i = I2cDAT;	// Le lixo no dado
			if(n == 0) {	// Se nao tem dado para ler
				I2cCONSET = 0x10;	// Gera stop
				i=-1;
				}
			if(n == 1) {	// Se so tem um dado
				I2cCONCLR = 4; // Gera NAK
				}
			else	I2cCONSET = 4;	// Se tem mais dados gera ACK
			i=0;
			I2cCONCLR=8;
			to = TOINI;
			break;
		case 0x58:	// Dado recebido com NACK
		case 0x50:	// Dado recebido e gerou ack
			buffer[i++] = I2cDAT;	// Recebe mais um dado
			if( i > n) {	// Se terminou gera STOP
				I2cCONSET = 0x10; 
				i=-1; 
				}
			if( i == n)	I2cCONCLR=4; // No ultimo dado gera NACK
			else I2cCONSET=4;	// Se nao e' ultimo gera ACK
			I2cCONCLR=8;
			to = TOINI;
			break;
		default: if(!to--) i=-1; break;
		}
	} while(i!=-1);
to = TOINI;
while((to--) && (I2cCONSET & 0x10));	// Espera terminar o stop bit
I2cCONCLR = 0x1f;
return !to;	// Saida com sucesso
}
