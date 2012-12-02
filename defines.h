#ifndef DEFINES
#define DEFINES

#define L1 0x80
#define L2 0xC0
#define L3 0x94
#define L4 0xD4
#define BTO1 !(FIO4PIN & 0x0100)
#define BTO2 !(FIO4PIN & 0x0200)
#define BTO3 !(FIO4PIN & 0x0400)
#define BTO4 !(FIO4PIN & 0x0800)
#define BTO5 !(FIO4PIN & 0x1000)
#define ALL_LED 0xFF
#define LED1 0x01
#define LED2 0x02
#define LED3 0x04
#define LED4 0x08
#define LED5 0x10
#define LED6 0x20
#define LED7 0x40
#define LED8 0x80

#define ON 1
#define OFF 0
#endif
