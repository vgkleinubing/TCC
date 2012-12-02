#ifndef _COMMFUNC
#define _COMMFUNC

#include "integer.h"

void uart0_init (void);
int uart0_test (void);
void uart0_put (BYTE);
BYTE uart0_get (void);
BYTE U0get(int t);


void uart1_init (void);
int uart1_test (void);
void uart1_put (BYTE);
BYTE uart1_get (void);
BYTE U1get(int t);

#endif

