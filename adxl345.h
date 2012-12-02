/*
 * Date: Sat 01/Dez/2012
 * PUC - ENGENHARIA DE COMPUTACAO
 * Autor: Vinicius Grubel Kleinubing
 * e-mail: vgkleinubing@gmail.com
 */

#ifndef ADXL345_h
#define ADXL345_h

/* Default address */
/* #define DefaultADXL345_Address 0x1D */
#define DefaultADXL345_Address 0x53
#define ADXL345write 0xA6
#define ADXL345read 0xA7


#define Register_Devid 0x00
#define Register_Xoffset 0x1E
#define Register_Yoffset 0x1F
#define Register_Zoffset 0x20
#define Register_BW_Rate 0x2C
#define Register_PowerControl 0x2D
#define Register_DataFormat 0x31
#define Register_DataX 0x32
#define Register_DataY 0x34
#define Register_DataZ 0x36

//#define ScaleFor2G 0.0039
#define ScaleFor4G 0.0078
#define ScaleFor8G 0.0156
#define ScaleFor16G 0.0312
#define ScaleFor2G 0.0156


int adxl345_init(int x_off, int y_off, int z_off);

int adxl345_get_x(void);
int adxl345_get_y(void);
int adxl345_get_z(void);
int adxl345_get_xyz(int *x, int *y, int *z);

#endif

