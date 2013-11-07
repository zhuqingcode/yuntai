#ifndef I2C_H

#define I2C_H
#include <AT89X52.h>
//#include <reg52.h>
#include <intrins.h>


#define  LC16_W 0xA0
#define  LC16_R 0xA1

#define  SCAN_ADDR 0x00
#define  RIGHT_ADDR 0x10
#define  LEFT_ADDR 0x20
#define  QCOL_ADDR 0x30
#define  PRESET_ADDR 0x100
#define  STATE_ADDR 0x40

#define  QU_STATE 0x1
#define  AUTO_STATE 0x2
#define  OTEHER_STATE 0x3

#define  STORE_LEN 8


sbit     SCL = P2^2;
sbit     SDA = P2^3;



unsigned char i2c_lc16_write(unsigned int addr,unsigned char *pbuf,unsigned int num);
unsigned char i2c_lc16_read(unsigned int addr,unsigned char *pbuf,unsigned int num);
void Set_Preset(unsigned int low,unsigned int col,unsigned char num);
void Clear_Preset(unsigned char num);
unsigned char Read_Preset(unsigned int *low,unsigned int *col,unsigned char num);
void Set_addr(unsigned int col,unsigned int addr);
unsigned char Read_addr(unsigned int *col,unsigned int addr);
void Set_State(unsigned char state,unsigned int addr);
unsigned char Read_State(unsigned char *state,unsigned int addr);
void Clear_State(unsigned int addr);
//void Clear_addr(unsigned int addr);
#endif   /*UART_H*/