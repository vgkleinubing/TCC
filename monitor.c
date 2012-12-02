#include <string.h>
#include <stdarg.h>
#include "lcd.h"
#include "monitor.h"

int xatoi (char **str, long *res)
{
	DWORD val;
	BYTE c, radix, s = 0;


	while ((c = **str) == ' ') (*str)++;
	if (c == '-') {
		s = 1;
		c = *(++(*str));
	}
	if (c == '0') {
		c = *(++(*str));
		if (c <= ' ') {
			*res = 0; return 1;
		}
		if (c == 'x') {
			radix = 16;
			c = *(++(*str));
		} else {
			if (c == 'b') {
				radix = 2;
				c = *(++(*str));
			} else {
				if ((c >= '0')&&(c <= '9'))
					radix = 8;
				else
					return 0;
			}
		}
	} else {
		if ((c < '1')||(c > '9'))
			return 0;
		radix = 10;
	}
	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;
		}
		if (c >= radix) return 0;
		val = val * radix + c;
		c = *(++(*str));
	}
	if (s) val = -val;
	*res = val;
	return 1;
}

void xputc (char c)
{
	uart0_put(c);
}

void x1putc (char c)
{
	uart1_put(c);
}

void xputs (const char* str)
{
while (*str) xputc(*str++);
}

void xitoa (long val, int radix, int len)
{
	BYTE c, r, sgn = 0, pad = ' ';
	BYTE s[20], i = 0;
	DWORD v;


	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	v = val;
	r = radix;
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	if (len > 20) return;
	do {
		c = (BYTE)(v % r);
		if (c >= 10) c += 7;
		c += '0';
		s[i++] = c;
		v /= r;
	} while (v);
	if (sgn) s[i++] = sgn;
	while (i < len)
		s[i++] = pad;
	do
		xputc(s[--i]);
	while (i);
}

void xprintf (const char* str, ...)
{
	va_list arp;
	int d, r, w, s, l;


	va_start(arp, str);

	while ((d = *str++) != 0) {
		if (d != '%') {
			xputc(d); continue;
		}
		d = *str++; w = r = s = l = 0;
		if (d == '0') {
			d = *str++; s = 1;
		}
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) w = -w;
		if (d == 'l') {
			l = 1;
			d = *str++;
		}
		if (!d) break;
		if (d == 's') {
			xputs(va_arg(arp, char*));
			continue;
		}
		if (d == 'c') {
			xputc((char)va_arg(arp, int));
			continue;
		}
		if (d == 'u') r = 10;
		if (d == 'd') r = -10;
		if (d == 'X') r = 16;
		if (d == 'b') r = 2;
		if (!r) break;
		if (l) {
			xitoa((long)va_arg(arp, long), r, w);
		} else {
			if (r > 0)
				xitoa((unsigned long)va_arg(arp, int), r, w);
			else
				xitoa((long)va_arg(arp, int), r, w);
		}
	}

	va_end(arp);
}

void put_dump (const BYTE *buff, DWORD ofs, int cnt)
{
	BYTE n;


	xprintf("%08lX ", ofs);
	for(n = 0; n < cnt; n++)
		xprintf(" %02X", buff[n]);
	xputc(' ');
	for(n = 0; n < cnt; n++) {
		if ((buff[n] < 0x20)||(buff[n] >= 0x7F))
			xputc('.');
		else
			xputc(buff[n]);
	}
	xputc('\n');
}

void get_line(char *s, int n)
{
int i;
static char hist[80];
char c;
i=0;
do	{
	c=xgetc();
	if(c==0x1b) {
		if(xgetc()==0x5b){
			xgetc();
			xputc('\r'); xputc('>');
			for(i=0; (s[i]=hist[i]);i++) {
				xputc(s[i]);
				if(i > 78) break;
				}
			c = xgetc();
			}
		}
	if(c==0x7f) c=8;
	if(c==8){
		if(i)	{
			xputc(c); i--;
			xputc(' '); xputc(8);
			}
		}
	else	{
		s[i++]=c; 
		xputc(c);
		}
	} while(c!='\n' && c!='\r' && i < n-1);
if(c=='\r') xputc('\n');
s[i]=0;
for(i=0; i < 78 && s[i]>=' '; i++) hist[i]=s[i];
hist[i]='\0';
}

int x1_get_line(char *s, int n)
{
int i = 0;
char c;
do	{
	c=x1getc();
	s[i++]=c; 
} while(c!='\n' && c!='\r' && i < n-1);
/* extra get to get LF */
c=x1getc();
s[i]=0;
return 0;
}
