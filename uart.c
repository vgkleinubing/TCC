#include <string.h>
#include "LPC2300.h"
#include "interrupt.h"
#include "uart.h"
#include "lcd.h"

#define BUFFER_SIZE 255

/* With this settings we get an exact value for 19200 Baud */
#define PCLK		72000000

#define U0_BPS 		19200
#define U1_BPS 		4800

#define	DIVADDVAL	7
#define	MULVAL		8

#define	U0_DLVAL		((PCLK*MULVAL/(MULVAL+DIVADDVAL)/U0_BPS+8)>>4)
#define	U1_DLVAL		((PCLK*MULVAL/(MULVAL+DIVADDVAL)/U1_BPS+8)>>4)

static volatile struct
{
	int		rptr;
	int		wptr;
	int		count;
	BYTE	buff[BUFFER_SIZE];
} TxFifo0, RxFifo0;
static volatile int TxRun0;

static volatile struct
{
	int		rptr;
	int		wptr;
	int		count;
	BYTE	buff[BUFFER_SIZE];
} TxFifo1, RxFifo1;
static volatile int TxRun1;

void Isr_UART0 (void)
{
	int d, idx, cnt, iir;

	do	{
		iir = U0IIR;			/* Get Interrupt ID*/
		if (iir & 1) break;		/* Exit if there is no interrupt */
		switch (iir & 6) {
		case 4:			/* Receive FIFO is half filled or timeout occured */
			idx = RxFifo0.wptr;
			cnt = RxFifo0.count;
			while (U0LSR & 0x01) {			/* Receive all data in the FIFO */
				d = U0RBR;
				if (cnt < BUFFER_SIZE) {	/* Store data if buffer is not full */
					RxFifo0.buff[idx++] = d;
					cnt++;
					if(idx == BUFFER_SIZE) idx = 0;
				}
			}
			RxFifo0.wptr = idx;
			RxFifo0.count = cnt;
			break;

		case 2:			/* Transmisson FIFO empty */
			cnt = TxFifo0.count;
			if (cnt) {
				idx = TxFifo0.rptr;
				for (d = 12; d && cnt; d--, cnt--) {	/* Store data into FIFO (max 12 chrs) */
					U0THR = TxFifo0.buff[idx++];
					if(idx == BUFFER_SIZE) idx = 0;
				}
				TxFifo0.rptr = idx;
				TxFifo0.count = cnt;
			} else {
				TxRun0 = 0;				/* When no data in the buffer, clear running flag */
			}
			break;

		default:		/* Data error or break detected */
			d = U0LSR;
			d = U0RBR;
			break;
		}
	} while(1);
}

int uart0_test (void)
{
	return RxFifo0.count;
}

BYTE uart0_get (void)
{
return U0get(-1);
}

extern volatile UINT Timer;

BYTE U0get (int mtempo)
{
	BYTE d;
	int idx;
	static int brilho=0x38;
	int k,seg;
	int tempo, tini;
	tini = Timer;
	tempo = mtempo / 1000;
	/* Wait while Rx buffer is empty */
	seg = RTC_SEC;
	k=10000;
	while (!RxFifo0.count) {
		idx = LEBOTAO;
		if((mtempo > 0) && (Timer-tini) >= mtempo) return ' ';
		switch(idx){
			case SW3: brilho += 2; 	// KB4 incrementa contraste
			case SW4: brilho --;	// KB5 decrementa contraste
			break;
			case SW2: if(k==0) return ' '; 
			break;
			case SW1: tempo=5; 
			break;
			default: if(k) k--; break;
			}
		if(tempo > 0 && seg!=RTC_SEC) {
			LCDcomando(0x86);
			LCDputchar(tempo+'0');
			LCDputchar(' ');
			LCDcomando(0x87);
			LCDputs(" T=");
			if((--tempo) == 0) return ' ';
			seg = RTC_SEC;
			}
		}

	U0IER = 0;				/* Disable interrupts */
	idx = RxFifo0.rptr;
	d = RxFifo0.buff[idx];	/* Get a byte from Rx buffer */
	RxFifo0.rptr = (idx + 1) % BUFFER_SIZE;
	RxFifo0.count--;
	U0IER = 0x07;			/* Enable interrupt */

	return d;
}

void uart0_put (BYTE d)
{
#if 0
	while (!(U0LSR & 0x20));
	U0THR = d;
#else
	int idx, cnt;

	/* Wait for buffer ready */
	while (TxFifo0.count >= BUFFER_SIZE);

	U0IER = 0x05;		/* Disable Tx Interrupt */
	if (!TxRun0) {		/* When not in runnig, trigger transmission */
		U0THR = d;
		TxRun0 = 1;
	} else {			/* When transmission is runnig, store the data into the Tx buffer */
		cnt = TxFifo0.count;
		idx = TxFifo0.wptr;
		TxFifo0.buff[idx] = d;
		TxFifo0.wptr = (idx + 1) % BUFFER_SIZE;
		TxFifo0.count = ++cnt;
	}
	U0IER = 0x07;		/* Enable Tx Interrupt */
#endif
}

void uart0_init (void)
{
	U0IER = 0x00;
	RegisterVector(UART0_INT, Isr_UART0, PRI_LOWEST, CLASS_IRQ);
	/* Set PCLKSEL0 to divide by one: U0PCLK = 72MHz */
	PCONP |= 8;		/* Turn UART0 on */
	PCLKSEL0 = (PCLKSEL0 & (~0xc0)) | 0x40;
	/* Attach UART0 unit to I/O pad */
	PINSEL0 = (PINSEL0 & (~0xf0)) | 0x50;
	/* Initialize UART0 */
	U0LCR = 0x83;		/* Select divisor latch */
	U0DLM = U0_DLVAL >> 8;	/* Initialize BRG */
	U0DLL = U0_DLVAL & 0xff;
	U0FDR = (MULVAL << 4) | DIVADDVAL;
	U0LCR = 0x03;		/* Set serial format N81 and deselect divisor latch */
	U0FCR = 0x87;		/* Enable FIFO */
	U0TER = 0x80;		/* Enable Tansmission */
	/* Clear Tx/Rx FIFOs */
	TxFifo0.rptr = 0;
	TxFifo0.wptr = 0;
	TxFifo0.count = 0;
	RxFifo0.rptr = 0;
	RxFifo0.wptr = 0;
	RxFifo0.count = 0;
	/* Enable Tx/Rx/Error interrupts */
	U0IER = 0x07;
}

/****************************************************************************
 *									UART1									*
 ****************************************************************************/

void Isr_UART1 (void)
{
	int d, idx, cnt, iir;

	do	{
		iir = U1IIR;			/* Get Interrupt ID*/
		if (iir & 1) break;		/* Exit if there is no interrupt */
		switch (iir & 6) {
		case 4:			/* Receive FIFO is half filled or timeout occured */
			idx = RxFifo1.wptr;
			cnt = RxFifo1.count;
			while (U1LSR & 0x01) {			/* Receive all data in the FIFO */
				d = U1RBR;
				if (cnt < BUFFER_SIZE) {	/* Store data if buffer is not full */
					RxFifo1.buff[idx++] = d;
					cnt++;
					if(idx == BUFFER_SIZE) idx = 0;
				}
			}
			RxFifo1.wptr = idx;
			RxFifo1.count = cnt;
			break;

		case 2:			/* Transmisson FIFO empty */
			cnt = TxFifo1.count;
			if (cnt) {
				idx = TxFifo1.rptr;
				for (d = 12; d && cnt; d--, cnt--) {	/* Store data into FIFO (max 12 chrs) */
					U1THR = TxFifo1.buff[idx++];
					if(idx == BUFFER_SIZE) idx = 0;
				}
				TxFifo1.rptr = idx;
				TxFifo1.count = cnt;
			} else {
				TxRun1 = 0;				/* When no data in the buffer, clear running flag */
			}
			break;

		default:		/* Data error or break detected */
			d = U1LSR;
			d = U1RBR;
			break;
		}
	} while(1);
}

int uart1_test (void)
{
	return RxFifo1.count;
}

BYTE uart1_get (void)
{
return U1get(-1);
}

BYTE U1get (int mtempo)
{
	BYTE d;
	int idx;
	while (!RxFifo1.count){};

	U1IER = 0;				/* Disable interrupts */
	idx = RxFifo1.rptr;
	d = RxFifo1.buff[idx];	/* Get a byte from Rx buffer */
	RxFifo1.rptr = (idx + 1) % BUFFER_SIZE;
	RxFifo1.count--;
	U1IER = 0x07;			/* Enable interrupt */

	return d;
}

void uart1_put (BYTE d)
{
#if 0
	while (!(U0LSR & 0x20));
	U1THR = d;
#else
	int idx, cnt;

	/* Wait for buffer ready */
	while (TxFifo1.count >= BUFFER_SIZE);

	U1IER = 0x05;		/* Disable Tx Interrupt */
	if (!TxRun1) {		/* When not in runnig, trigger transmission */
		U1THR = d;
		TxRun1 = 1;
	} else {			/* When transmission is runnig, store the data into the Tx buffer */
		cnt = TxFifo1.count;
		idx = TxFifo1.wptr;
		TxFifo1.buff[idx] = d;
		TxFifo1.wptr = (idx + 1) % BUFFER_SIZE;
		TxFifo1.count = ++cnt;
	}
	U1IER = 0x07;		/* Enable Tx Interrupt */
#endif
}

void uart1_init (void)
{
	U1IER = 0x00;
	RegisterVector(UART1_INT, Isr_UART1, PRI_LOWEST, CLASS_IRQ);
	/* Set PCLKSEL0 to divide by one: U0PCLK = 72MHz */
	PCONP |= 0x10;		/* Turn UART1 on */
	PCLKSEL0 = (PCLKSEL0 & (~0x30)) | 0x100;
	/* Attach UART1 unit to I/O pad */
	PINSEL0 |= 0x40000000; /* Enable TxD1 P0.15 */
	PINSEL1 |= 0x00000001; /* Enable RxD1 P0.16 */
	/* Initialize UART0 */
	U1LCR = 0x83;		/* Select divisor latch */
	U1DLM = U1_DLVAL >> 8;	/* Initialize BRG */
	U1DLL = U1_DLVAL & 0xff;
	U1FDR = (MULVAL << 4) | DIVADDVAL;
	U1LCR = 0x03;		/* Set serial format N81 and deselect divisor latch */
	U1FCR = 0x87;		/* Enable FIFO */
	U1TER = 0x80;		/* Enable Tansmission */
	/* Clear Tx/Rx FIFOs */
	TxFifo1.rptr = 0;
	TxFifo1.wptr = 0;
	TxFifo1.count = 0;
	RxFifo1.rptr = 0;
	RxFifo1.wptr = 0;
	RxFifo1.count = 0;
	/* Enable Tx/Rx/Error interrupts */
	U1IER = 0x07;
}
