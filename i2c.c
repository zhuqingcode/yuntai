#include "i2c.h"


void I2cDelay(void)
{
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

void delay_1ms()
{
   unsigned char i=500;
   for(;i>0;i--)
   {
      I2cDelay();
   }
}


void I2cStart (void)
{
    SDA = 1;
    SCL = 1;
    I2cDelay();
    SDA = 0;
    I2cDelay();
    SCL = 0;
    I2cDelay();
}

void I2cStop(void)
{
   SCL = 0;
   SDA = 0;
   I2cDelay();
   SCL = 1;
   I2cDelay();
   SDA = 1;
   I2cDelay();
   I2cDelay();
}

void I2cAck (void)
{
    I2cDelay();
    SDA = 0;
    I2cDelay();
    SCL = 1;
    I2cDelay();
    SCL = 0;
    I2cDelay();
    SDA = 1; 
    I2cDelay();       
}


void I2cNak (void)
{
    I2cDelay();
    SDA = 1;
    I2cDelay();    
    SCL = 1;
    I2cDelay();
    SCL = 0; 
    I2cDelay();
    SDA = 1; 
    I2cDelay();       
}

unsigned char I2c_RcvByte(void)
{
    unsigned char i, buf;
    
    I2cDelay();
    SDA = 1;
    
    for(i = 0; i < 8; i++)
    {
        I2cDelay();
        SCL = 1;
        I2cDelay();
        buf <<= 1;
        	
        if(SDA)
        {
            buf |= 0x01; 
        }      
        
        SCL = 0;   
    }

    return (buf);
}

bit I2c_SendByte(unsigned char dat)
{
    unsigned char i;
    bit ack = 0;
 
	for(i = 0; i < 8; i++)
	{
		if(dat & 0x80)
        {
            SDA = 1;   
        } 
		else
        {
            SDA = 0;  
        }
    		    
		dat <<= 1;
        I2cDelay();
		SCL = 1;
        I2cDelay();
        SCL = 0;
        I2cDelay();
	}

    SDA = 1;
    I2cDelay();    
    SCL = 1;
    I2cDelay();
    ack = SDA;
    SCL = 0;
    I2cDelay();
    
    return(ack);
}

unsigned char i2c_lc16_write(unsigned int addr,unsigned char *pbuf,unsigned int num)
{
    unsigned char addrl, addrh,i;
    bit tmp=0;
    EA=0;
    addrl = addr&0xff;
    addrh = (addr>>7)&0x0e;
    I2cStart();
    tmp=I2c_SendByte(LC16_W|addrh);
    if(tmp ==1)
    {
       I2cStop();
       return 1;
    }
    tmp=I2c_SendByte(addrl);
    if(tmp ==1)
    {
       I2cStop();
       return 1;
    }
    for(i=0;i<num;i++)
    {
       tmp=I2c_SendByte(*pbuf);
       if(tmp ==1)
       {
         I2cStop();
         return 1;
       }
       pbuf++;
    }
    I2cStop();
    delay_1ms();
    EA=1;
    return 0;

}
unsigned char i2c_lc16_read(unsigned int addr,unsigned char *pbuf,unsigned int num)
{
    unsigned char addrl, addrh,i;
    bit tmp=0;
    EA=0;
    addrl = addr&0xff;
    addrh = (addr>>7)&0x0e;
    I2cStart();
    tmp=I2c_SendByte(LC16_W);
    if(tmp ==1)
    {
       I2cStop();
       return 1;
    }
    tmp=I2c_SendByte(addrl);
    if(tmp ==1)
    {
       I2cStop();
       return 1;
    }
    I2cStart();
    tmp=I2c_SendByte(LC16_R|addrh);
    if(tmp ==1)
    {
       I2cStop();
       return 1;
    }
    for(i=0;i<num-1;i++)
    {
       *pbuf = I2c_RcvByte();
       pbuf++;
       I2cAck();
    }
    *pbuf = I2c_RcvByte();
    I2cNak();
    I2cStop();
    delay_1ms();
    EA=1;
    return 0;
}

void Set_Preset(unsigned int low,unsigned int col,unsigned char num)
{
    unsigned char  pbuf[5];
    unsigned int addr;
    pbuf[0] = 0x1;
    pbuf[1] = low>>8;
    pbuf[2] = low&0xff;
    pbuf[3] = col>>8;
    pbuf[4] = col&0xff;
    addr = PRESET_ADDR+num*STORE_LEN;
    i2c_lc16_write(addr,pbuf,5);
}

void Clear_Preset(unsigned char num)
{
    unsigned char  pbuf[5];
    unsigned int addr;
    pbuf[0] = 0xff;
    pbuf[1] = 0xff;
    pbuf[2] = 0xff;
    pbuf[3] = 0xff;
    pbuf[4] = 0xff;
    addr = PRESET_ADDR+num*STORE_LEN;
    i2c_lc16_write(addr,pbuf,5);
}

unsigned char Read_Preset(unsigned int *low,unsigned int *col,unsigned char num)
{
    unsigned char  pbuf[5];
    unsigned int addr;
    addr = PRESET_ADDR+num*STORE_LEN;
    i2c_lc16_read(addr,pbuf,5);
    if(pbuf[0]==0x1)
    {
       *low = pbuf[1]<<8|pbuf[2];
       *col = pbuf[3]<<8|pbuf[4];
       return 0;
    }
    else
    {
       return 1;
    }

}

void Set_addr(unsigned int col,unsigned int addr)
{
    unsigned char  pbuf[5];
    pbuf[0] = 0x1;
    pbuf[1] = col>>8;
    pbuf[2] = col&0xff;
    pbuf[3] = 0x0;
    pbuf[4] = 0x0;
    i2c_lc16_write(addr,pbuf,5);
}



/*void Clear_addr(unsigned int addr)
{
    unsigned char  pbuf[5];
    pbuf[0] = 0x0;
    pbuf[1] = 0x0;
    pbuf[2] = 0x0;
    pbuf[3] = 0x0;
    pbuf[4] = 0x0;
    i2c_lc16_write(addr,pbuf,5);
}  */

 unsigned char Read_addr(unsigned int *col,unsigned int addr)
{
    unsigned char  pbuf[5];
    i2c_lc16_read(addr,pbuf,5);
    if(pbuf[0]==0x1)
    {
       *col = pbuf[1]<<8|pbuf[2];
       return 0;
    }
    else
    {
       return 1;
    }

}


void Set_State(unsigned char state,unsigned int addr)
{
    unsigned char  pbuf[5];
    pbuf[0] = 0x1;
    pbuf[1] = state;
    i2c_lc16_write(addr,pbuf,5);
}

 unsigned char Read_State(unsigned char *state,unsigned int addr)
{
    unsigned char  pbuf[5];
    i2c_lc16_read(addr,pbuf,5);
    if(pbuf[0]==0x1)
    {
       *state = pbuf[1];
       return 0;
    }
    else
    {
       return 1;
    }
}

/*void Clear_State(unsigned int addr)
{
    unsigned char  pbuf[5];
    pbuf[0] = 0x0;
    pbuf[1] = 0x0;
    pbuf[2] = 0x0;
    pbuf[3] = 0x0;
    pbuf[4] = 0x0;
    i2c_lc16_write(addr,pbuf,5);
} */

