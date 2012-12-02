/*
 * Date: Sat 01/Dez/2012
 * PUC - ENGENHARIA DE COMPUTACAO
 * Autor: Vinicius Grubel Kleinubing
 * e-mail: vgkleinubing@gmail.com
 */

#include <string.h>
#include "LPC2300.h"
#include "interrupt.h"
#include "uart.h"
#include "i2c.h"
#include "lcd.c"
#include "monitor.h"
#include "adxl345.h"
#include "mprintf.h"

/* acelerometer variables */
char buf[16];

int adxl345_init(int x_off, int y_off, int z_off)
{

/* Teste inicial do I2c */
	buf[0] = Register_Devid;
	escreve_i2c(ADXL345write, buf, 1);
	escreve_i2c(ADXL345read, buf, 1);
//	xprintf("Buffer device ID: 0x%X\n", buf[0]);
//	xprintf("Configure offset x%d y%d z%d\n", x_off, y_off, z_off);

	if (buf[0] != 0xE5)
		return -1;
	
	/* Disable sensor before configure */
    buf[0] = Register_PowerControl;
    buf[1] = 0x00;
    escreve_i2c(ADXL345write, buf, 2);

/* Select range to 2g*/
	buf[0] = Register_DataFormat;

	/*Set range for 2g and full resolution*/
	buf[1] = 0x00 | 0x08;

	/*Set range for 4g and full resolution*/
//	buf[1] = 0x01 | 0x08;

	/*Set range for 8g and full resolution*/
//	buf[1] = 0x02 | 0x08;

	/*Set range for 16g and full resolution*/
//	buf[1] = 0x03 | 0x08;

/* Enable self test*/
//	buf[1] |= 0x80;
	escreve_i2c(ADXL345write, buf, 2);

/* Clear ofset */
	buf[0] = Register_Xoffset;
	buf[1] = -(x_off/4);
	buf[2] = -(y_off/4);
	buf[3] = -((z_off-256))/4;
	escreve_i2c(ADXL345write, buf, 2);

/*	Set to 800Hz 0D */
/*	Set to 1600Hz 0F*/
	buf[0] = Register_BW_Rate;
//	buf[1] = 0x0D;
	buf[1] = 0x0F;
	escreve_i2c(ADXL345write, buf, 2);

/* Enable sensor
 * POWER SEQUENCE - Page 6
 * First configure sensor, after enable
 */
    buf[0] = Register_PowerControl;
    buf[1] = 0x08;
    escreve_i2c(ADXL345write, buf, 2);

	return 0;
}

int adxl345_get_x()
{
	short int x;

	buf[0] = Register_DataX;
	escreve_i2c(ADXL345write, buf, 1);
	escreve_i2c(ADXL345read, buf, 2);

	x = (buf[1] << 8) | buf[0];

	return (int)x;
}

int adxl345_get_y()
{
	short int y;

	buf[0] = Register_DataY;
	escreve_i2c(ADXL345write, buf, 1);
	escreve_i2c(ADXL345read, buf, 2);

	y = (buf[1] << 8) | buf[0];

	return (int)y;
}

int adxl345_get_z()
{
	short int z;

	buf[0] = Register_DataZ;
	escreve_i2c(ADXL345write, buf, 1);
	escreve_i2c(ADXL345read, buf, 2);

	z = (buf[1] << 8) | buf[0];

	return (int)z;
}

int adxl345_get_xyz(int *x, int *y, int *z)
{
	short int xr, yr, zr;

	buf[0] = Register_DataX;
	escreve_i2c(ADXL345write, buf, 1);
	escreve_i2c(ADXL345read, buf, 6);

	xr = (buf[1] << 8) | buf[0];
	yr = (buf[3] << 8) | buf[2];
	zr = (buf[5] << 8) | buf[4];

	*x = (int)xr;
	*y = (int)yr;
	*z = (int)zr;

	return 0;
}
