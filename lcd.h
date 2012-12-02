/******************************************
Cabecalho da configuracao do LCD
Estas definicoes procuram abstrair as diferencas de portas I/O
dos kits de Lab. Processadores
A definicao da palavra MODELO esta' no Makefile
M. Stemmer 2011/1
******************************************/
#ifndef LDC_h
#define LCD_h


#if MODELO == 2008
#define LCD_DIR	FIO4DIR|=0x03000000
#define SET_E	FIO4SET3=0x01
#define CLR_E	FIO4CLR3=0x01
#define SET_RS	FIO4SET3=0x02
#define CLR_RS	FIO4CLR3=0x02
#define SW1 0x008
#define SW2 0x040
#define SW3 0x200
#define SW4 0x400
#define LEBOTAO ((~FIO1PINU) & (SW1 | SW2| SW3 | SW4))
#endif

#if MODELO == 2009
#define LCD_DIR	FIO4DIR|=0x03000000
#define SET_E	FIO4SET3=0x01
#define CLR_E	FIO4CLR3=0x01
#define SET_RS	FIO4SET3=0x02
#define CLR_RS	FIO4CLR3=0x02
#define SW1 0x100
#define SW2 0x200
#define SW3 0x400
#define SW4 0x800
#define SW5 0x1000
#define LEBOTAO ((~FIO4PIN) & (SW1 | SW2| SW3 | SW4))
#endif

#if MODELO == 2010
#define LCD_DIR	FIO4DIR|=0x43000000
#define SET_E	FIO4SET3=0x01
#define CLR_E	FIO4CLR3=0x01
#define SET_RS	FIO4SET3=0x02
#define CLR_RS	FIO4CLR3=0x02
#define SW1 0x40
#define SW2 (1<<18)
#define SW3 0x100
#define SW4 0x200
#define LEBOTAO ((((~FIO4PIN) & 0x300) | ((~FIO0PIN) & SW2)) | (((~FIO2PIN) & 0x180)>>1))
#endif

#if MODELO == 2011
#define LCD_DIR	FIO2DIR|=0x0c
#define SET_E	FIO2SET=8
#define CLR_E	FIO2CLR=8
#define SET_RS	FIO2SET=4
#define CLR_RS	FIO2CLR=4
#define SW1 0x10
#define SW2 0x20
#define SW3 0x40
#define SW4 0x80
#define SW5 0x100
#define LEBOTAO ((~FIO2PIN) & (SW1 | SW2| SW3 | SW4))
#endif
/* Endereco i2c do LM75A */
#define ADDRTMP 0x90

/* Modulo lcd.c */
void LCDinit(void);
void LCDcomando(int c);
void LCDputchar(char c);
void LCDputs(char *s);
void espera(int t);

void LCDput2dec(int val);
void LCDputfloat(float val);
void LCDputpartint(int val);
void LCDputnum(int val);
void LCDp(int pos, char *txt);

#endif

