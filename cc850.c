/*----------------------------------------------------------------------*/
/* Unicode - OEM code bidirectional converter  (C)ChaN, 2009		*/
/* CP932 (Japanese Shift-JIS)						*/
/* Erased almost everything. No support for Japanese. (Stemmer)		*/
/*----------------------------------------------------------------------*/

#include "ff.h"

WCHAR inline ff_convert (	/* Converted code, 0 means conversion error */
	WCHAR	src,	/* Character code to be converted */
	UINT	dir		/* 0: Unicode to OEMCP, 1: OEMCP to Unicode */
)
{
	return src;
}



WCHAR inline ff_wtoupper (	/* Upper converted character */
	WCHAR chr		/* Input character */
)
{
if(chr >= 'a' && chr <= 'z') chr ^= ('a' ^ 'A');
return chr;
}
