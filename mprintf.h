#ifndef MPRINTF_H
#define MPRINTF_H

int atoi(char *str);
double atod(char *str);
int dprint(double x, int campo, int frac, void (*putc)(char));
int sprintf(char *buf, const char *formato, ... );

#endif /* MPRINTF_H */

