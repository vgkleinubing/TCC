/*******************************************************
Modulo para usar o I2C 
Prototipos do modulo i2c.c
Marcos A. Stemmer
********************************************************/
#define CRYSTALFREQ 12000000
/* Definindo USA_I2C0 as rotinas vao configurar para usar o i2c0
   Tambem pode-se definir USA_I2C2 para usar o i2c2 */
//#define USA_I2C0

/*
#if MODELO == 2011
#define USA_I2C0
#else
#define USA_I2C2
#endif
*/
#define USA_I2C0


#ifdef USA_I2C0
#define	I2C_BASE_ADDR	0xE001C000
#endif

#ifdef USA_I2C2
#define	I2C_BASE_ADDR	0xE0080000
#endif

#define	I2cCONSET      (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x00))
#define	I2cSTAT	       (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x04))
#define	I2cDAT	       (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x08))
#define	I2cADR	       (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x0C))
#define	I2cSCLH	       (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x10))
#define	I2cSCLL	       (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x14))
#define	I2cCONCLR      (*(volatile unsigned long *)(I2C_BASE_ADDR + 0x18))
/* Inicializa a interface i2c2 com interrupcao. 
Deve ser chamada 1 vez no programa antes de comecar a usar o i2c.*/
void ini_i2c(void);

/* Escreve dados em um dispositivo i2c
i2caddr	Endereco i2c do dispositivo
buf	Local onde os dados estao armazenados
n	Numero de bytes que devem ser escritos
Retorna ZERO em caso de sucesso */
int escreve_i2c(int i2caddr, char *buf, int n);	

/* Le dados de um dispositivo i2c
i2caddr	Endereco i2c do dispositivo
buf	Local onde deve armazenar os dados lidos
n	Numero de bytes que devem ser lidos
Retorna ZERO se conseguiu ler */
#define le_i2c(a,b,n) escreve_i2c((a)|1,b,n)
