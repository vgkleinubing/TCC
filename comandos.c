/*******************************************************
  Interpretador de comandos para acessar o cartao sd-mmc
  Este modulo foi escrito por Marcos A. Stemmer
  Adaptado para o kit lpc2378-2011 de Sistemas Embarcados: FENG-PUCRS
  Usa a bilioteca de FAT flilesystem e acesso a MMC (C)ChaN, 2009
  [http://elm-chan.org/docs/mmc/mmc_e.html]
 ********************************************************/
#include <string.h>
#include "LPC2300.h"
#include "integer.h"
#include "interrupt.h"
#include "uart.h"
#include "monitor.h"
#include "rtc.h"
#include "diskio.h"
#include "ff.h"
#define MEMCHECK
#include "lcd.h"
#include "i2c.h"
#include "comandos.h"

#if _FS_MINIMIZE <= 1
int lsdir(char *);
#endif
int type(char *);
int ft_get(char *, char *);
int ft_put(char *, char *);

FATFS Fatfs[_VOLUMES];

FILINFO Finfo;
#if _USE_LFN
char Lfname[512];
#endif
#define BUFLEN 16384
BYTE Buff[BUFLEN] __attribute__ ((aligned (4))) ;
typedef void (*FPTR)(void);

void put_rc (int rc)
{
	const char *p;
	static const char str[] =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && *p; i++) {
		while(*p++);
	}
	xprintf("rc=%u FR_%s\n", (UINT)rc, p);
}

/* Hashing function para compactar as palavras de comando */
unsigned long hash(char *cmd)
{
	unsigned long hs=0;
	while(*cmd){
		hs = (hs << 4) + hs + *cmd;
		cmd++;
	}
	return hs;
}

/* Separa palavras da linha de comando */
char *separa(char *s)
{
	if (*s == '\"') {
		s++;
		while(*s >= ' ' && *s <= 'z' && *s != '\"') s++;
		*s = '\0';
		s++;
	}
	else {
		while(*s > ' ' && *s <= 'z') s++;
		*s++ = '\0';
	}
	while(*s && (*s <= ' ')) s++;
	return s;
}

/* Remove o primeiro char se for " */
char *confere(char *s)
{
	if (*s == '\"') {
		s++;
	}
	return s;
}

char *extension(char *s)
{
	char *p;
	for(p=s; *s; s++) if(*s=='.') p=s+1;
	return p;
}

char *sonome(char *str)
{
	char *p;
	for(p = str; *str; str++) if((*str=='/') || (*str=='\\') || (*str==':')) p = str+1;
	return p;
}

/* Interpreta uma linha de comando */
void comando(char *cml)
{
	char *p1, *p2, *p3;
	unsigned long addr1, cnt;
	static const char *monthname[]={"jan","feb","mar","apr","may","jun", "jul", "aug", "sep", 
		"oct","nov", "dec"};
	RTC rtc;
	if(cml==NULL) return;
	strcpy((char *)Buff, cml);
	cml = (char *)Buff;
	for(; *cml==' ' || *cml=='\t'; cml++);
	if(!(*cml)) return;
	p1 = separa(cml);
	p2 = separa(p1);
	p3 = separa(p2);
	p2 = confere(p2);
	switch(hash(cml)){
#if _FS_MINIMIZE <= 2
		case 0x8cd2448: // append
			xputs("append <file> string\t\tAppend in end of file a string \n");
			xprintf("cml:%s \n p1:%s \np2:%s\n p3:%s\n", cml, p1, p2, p3);
			append(p1, p2);
			break;
#endif
		case 0x7b70: // get
			if(*p2) put_rc(ft_get(p1,p2));
			else put_rc(ft_get(p1, sonome(p1)));
			break;
		case 0x86a9: // put
			if(*p2) put_rc(ft_put(p1,p2));
			else put_rc(ft_put(p1, sonome(p1)));
			break;
		case 0x8d1a: // vt0
			xprintf("T0PR = %d\n", T0PR); break;
#if _FS_RPATH >= 1
		case 0x06F7:	// cd <path>	=> Change directory
			put_rc(f_chdir(p1));
			break;
#endif
#if _FS_MINIMIZE <= 1
		case 0x784f: // dir
		case 0x079F:	// ls [<path>]	=> List directory
			lsdir(p1);
			break;
#endif
#if _FS_MINIMIZE == 0
		case 0x07FF:	// rm <file>	=> Remove (delete) file
			put_rc(f_unlink(p1));
			break;
#endif
		case 0x942a2: // type
		case 0x76A8:	// cat <file>	=> Type file listing
			type(p1); break;
#if _FS_RPATH >= 2
		case 0x86bb: // pwd
			cnt = f_getcwd ((char *)Buff, 512);
			if(cnt) put_rc(cnt);
			else xprintf("%s\n",Buff);
			break;
#endif
		case 0x92F493:	// mdump <address> <count>	=> Memory dump
			if (!xatoi(&p1, (long *)&addr1)) break;
			if (!xatoi(&p2, (long *)&cnt)) cnt = 128;
			for (p3=(char*)addr1; cnt >= 16; p3 += 16, cnt -= 16)
				put_dump((BYTE*)p3, (UINT)p3, 16);
			if (cnt) put_dump((BYTE*)p3, (UINT)p3, cnt);
			break;
#if _FS_MINIMIZE == 0
		case 0x936777:	// mkdir <path>			=> Make directory
			put_rc(f_mkdir(p1));
			break;
#endif
		case 0x93C7C3:	// mount	=> Mount mmc device and filesystem
			xprintf("rc=%d\n", (WORD)disk_initialize(0));
			put_rc(f_mount(0, &Fatfs[0]));
			break;
#if _FS_MINIMIZE == 0
		case 0x07B3: // mv <oldname> <newname>
		case 0x87D5: // ren <oldname> <newname>
			put_rc(f_rename(p1, p2));
			break;
#endif
		case 0x7f4be: // date [<year> <mon> <mday> <hour> <min> <sec>]
			if (xatoi(&p1, (long *)&cnt)) {
				rtc.year = (WORD)cnt;
				xatoi(&p2, (long *)&cnt); rtc.month = (BYTE)cnt;
				p1=separa(p3);
				xatoi(&p3, (long *)&cnt); rtc.mday = (BYTE)cnt;
				p2=separa(p1);
				xatoi(&p1, (long *)&cnt); rtc.hour = (BYTE)cnt;
				p3=separa(p2);
				xatoi(&p2, (long *)&cnt); rtc.min = (BYTE)cnt;
				if (!xatoi(&p3, (long *)&cnt)) break;
				rtc.sec = (BYTE)cnt;
				rtc_settime(&rtc);
			}
			rtc_gettime(&rtc);
			xprintf("%u %s %u %02u:%02u:%02u\n", rtc.year, monthname[rtc.month-1],
					rtc.mday, rtc.hour, rtc.min, rtc.sec);
			break;
		case 0x84589: // help
			xputs(
					"append <file> string\t\tAppend in end of file a string \n"
					"cd <path>\t\t\t=> Change directory\n"
					"cp <src> <dest>\t\t\t=> Copy file\n"
					"date [<year> <mon> <mday> <hour> <min> <sec>]\n"
					"ls [<path>]\t\t\t=> List directory\n"
					"rm <file>\t\t\t=> Remove (delete) file\n"
					"mdump <address> <count>\t\t=> Memory dump\n"
					"mkdir <path>\t\t\t=> Make directory\n"
					"mount\t\t\t\t=> Mount mmc device and filesystem\n"
					"mv <oldname> <newname>\t\t=> (same as ren) Move or rename a file\n"
					"pwd\t\t\t\t=> Print working directory\n"
					"put <file>\t\t\t=> Upload a file to terminal\n");
			break;
		default:
			xputs("Unknown command. Try \"help\"\n");
			break;
	}
}

#if _FS_MINIMIZE <= 1
/* List a directory */
int lsdir(char *ptr)
{
	DIR Dir;	/* Directory object */
	long p1, p2;
	BYTE res;
	FATFS *fs;	/* Pointer to file system object */
	UINT s1, s2;
	res = f_opendir(&Dir, ptr);
	if (res) { put_rc(res); return res; }
	p1 = s1 = s2 = 0;
	do	{
#if _USE_LFN
		Finfo.lfname = Lfname;
		Finfo.lfsize = sizeof(Lfname);
#endif
		res = f_readdir(&Dir, &Finfo);
		if ((res != FR_OK) || !Finfo.fname[0]) break;
		if (Finfo.fattrib & AM_DIR) s2++;
		else	{
			s1++; p1 += Finfo.fsize;
		}
		xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
				(Finfo.fattrib & AM_DIR) ? 'D' : '-',
				(Finfo.fattrib & AM_RDO) ? 'R' : '-',
				(Finfo.fattrib & AM_HID) ? 'H' : '-',
				(Finfo.fattrib & AM_SYS) ? 'S' : '-',
				(Finfo.fattrib & AM_ARC) ? 'A' : '-',
				(Finfo.fdate >> 9) + 1980,
				(Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
				(Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
				Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
		for (p2 = strlen(Finfo.fname); p2 < 14; p2++) xputc(' ');
		xprintf("%s\n", Lfname);
#else
		xputc('\n');
#endif
	} while(1);
	xprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
#if _FS_MINIMIZE == 0
	if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
		xprintf(", %10lu bytes free\n", p1 * fs->csize * 512);
#endif
	return 0;
}
#endif

#if _FS_MINIMIZE <= 2
int append(char *filename, char *string)
{
	FIL File;
	int res;
	unsigned n;

	/* Open file for read and write */
	res = f_open(&File, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if (res) { put_rc(res); return res; }

	/* Pointer ro end of file */
	res = f_lseek(&File, f_size(&File));

	/* Write data */
	res = f_write(&File, string, strlen(string), &n);
	if (res) { put_rc(res); return res; }

	/* Close file */
	f_close(&File);
	if (res) put_rc(res);
	return res;
}
#endif

/* Dump a text file to the UART */
int type(char *filename)
{
	FIL File1;
	int res, i;
	unsigned n;
	xprintf("Opening \"%s\"", filename);
	res = f_open(&File1, filename, FA_OPEN_EXISTING | FA_READ);
	xputc('\n');
	if (res) { put_rc(res); return res; }
	do	{
		res = f_read(&File1, Buff, BUFLEN, &n);
		if (res) break;   /* error or eof */
		for(i = 0; i < n; i++) xputc(Buff[i]);
	} while(n == BUFLEN);
	f_close(&File1);
	if (res) put_rc(res);
	xputc('\n');
	return res;
}

static char const errorem[]="Remote error: %d\n";
#define TAMBLOCO 240
int ft_get(char *nomearq, char *dest)
{
	FIL File1;
	int res, err;
	unsigned i, k, n;
	xprintf("Geting \"%s\" -> \"%s\"\n", nomearq, dest);
	xputc('\xe0'); xputc('A');
	xputc(strlen(nomearq));
	xputs(nomearq);
	err = xgetc();
	n = xgetc();
	if(err) {
		xprintf(errorem, err);
		return 14;
	}

	res = f_open(&File1, dest, FA_CREATE_ALWAYS | FA_WRITE);
	if (res) { put_rc(res); return res; }
	do	{
		k = 0;
		do	{
			xputc('\xe0'); xputc('R');
			xputc(TAMBLOCO);
			err = xgetc();
			n = xgetc();
			if(err) break;
			for(i=0; i < n; i++) {
				Buff[k++] = xgetc();
			}
		} while((k < BUFLEN-TAMBLOCO) && (n == TAMBLOCO));
		res = f_write(&File1, Buff, k, &i);
	} while(!res && (n == TAMBLOCO) && !err);
	if(err) xprintf(errorem, err);
	xputc('\xe0'); xputc('F');
	xputc(0);
	err = xgetc();
	n = xgetc();
	f_close(&File1);
	return res;
}

int ft_put(char *nomearq, char *dest)
{
	FIL File1;
	int res, err;
	unsigned i, n;
	xprintf("Sending \"%s\" -> \"%s\"\n", nomearq, dest);
	xputc('\xe0'); xputc('B');
	xputc(strlen(dest));
	xputs(dest);
	err = xgetc();
	n = xgetc();
	if(err) {
		xprintf(errorem, err);
		return 14;
	}

	res = f_open(&File1, nomearq, FA_OPEN_EXISTING | FA_READ);
	if (res) { put_rc(res); return res; }
	do	{
		res = f_read(&File1, Buff, TAMBLOCO, &n);
		if (res) break;   /* error or eof */
		xputc('\xe0'); xputc('W');
		xputc(n);
		if(err) break;
		for(i = 0; i < n; i++) xputc(Buff[i]);
		err = xgetc();
		n = xgetc();
	} while(!res && (n == TAMBLOCO));
	if(err) xprintf(errorem, err);
	xputc('\xe0'); xputc('F');
	xputc(0);
	err = xgetc();
	n = xgetc();
	f_close(&File1);
	return res;
}
