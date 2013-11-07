
#include "serial.h"
unsigned  char idata  Serial_Rcvbuf[Serial_BufNum];//Serial_BufNum : 24
unsigned  char idata  Serial_Sendbuf[Serial_BufNum_W];//Serial_BufNum_W	: 8
unsigned char  *Serial_Rcv_Read = Serial_Rcvbuf;
unsigned char  *Serial_Send_Read = Serial_Sendbuf;
unsigned char  *Serial_Rcv_Write = Serial_Rcvbuf;
unsigned char  *Serial_Send_Write = Serial_Sendbuf;
unsigned int Rcv_Num=0;
unsigned int Send_Num=0;
unsigned int b_FlagSendOK=0;
extern unsigned char protocol;      //协议类型
extern unsigned char boundrate;     //波特率
extern unsigned char adress;        //地址
void InitUart()
{
    PCON = 0;
	SCON = 0;
	SCON |= 0x40;      // 10 bit (8, N, 1)

	SCON |= 0x10;       //REN置位，允许接收
	T2CON = 0x30;      // Timer2 for SBUF0 Baudrate	
    if(boundrate == BOUNDRATE_4800)
    {
      RCAP2H = 0xFF;     
	  RCAP2L = 0x64;
    }
    else
    {
      RCAP2H = 0xFF;     
	  RCAP2L = 0xB2;
    }
	TR2    = 1; 
    EA= 1;
	ES = 1;
}

void serial () interrupt 4 using 1
{
    if(RI)
    {
        RI = 0;
        
        *Serial_Rcv_Write = SBUF;//从SBUF中读取一个字节     
        if(Serial_Rcv_Write == (Serial_Rcvbuf + Serial_BufNum - 1))	//检测接受BUF是否满，如果满了，就把BUF的首地址重新赋值给Serial_Rcv_Write
        {															//也就是数据从BUF的第一个字节开始存放
            Serial_Rcv_Write = Serial_Rcvbuf;            
        }
		else 
        {
            Serial_Rcv_Write++;
        }
		
    
        Rcv_Num++;
                	
    }

    if(TI)
    {
        TI = 0;
        
        if(Send_Num == 0)
        {
            b_FlagSendOK = 0;                	// Send Finish 
        }
        else 
        {
            SBUF = *Serial_Send_Read;

            if(Serial_Send_Read == (Serial_Sendbuf + Serial_BufNum_W - 1)) //同上
            {
                Serial_Send_Read = Serial_Sendbuf;                        
            }
            else 
            {
                Serial_Send_Read++;
            }
        
            Send_Num--;
        }
    }
}


void PushSend (unsigned char byte) 
{
	ES = 0;
    *Serial_Send_Write = byte;
    
    if(Serial_Send_Write == (Serial_Sendbuf + Serial_BufNum_W - 1))   // 指针运算 越界后重新指向缓冲区首字节
    {
        Serial_Send_Write = Serial_Sendbuf;        
    }
    else 
    {
        Serial_Send_Write++;       // 调整队列指针 等待下一数据入队
    }

    Send_Num++;             // 待发数据计数器增1 
    
    if(b_FlagSendOK == 0)        // 若上次数据发送完毕 提请新的发送
    {
        b_FlagSendOK = 1;
        TI = 1;
    }

    ES = 1;
}

unsigned char PopRcv(void)
{
    unsigned char ch;
  	//ES = 0;
    
    ch = *Serial_Rcv_Read;      // 从收数缓冲中读取一字节数据 
    *Serial_Rcv_Read=0x00;	 
     if(Serial_Rcv_Read == (Serial_Rcvbuf + Serial_BufNum - 1))
     {
         Serial_Rcv_Read = Serial_Rcvbuf;                  
     }
     else 
     {
         Serial_Rcv_Read++;     // 调整指针 若越界则重新指向队列首地址 
     }

     Rcv_Num--;          // 接收数据计数器减1 
	  //	ES = 1;
     return (ch);         // 将弹出值返回
     
}


unsigned char Serial_Rcv(unsigned char *databuf)
{
   unsigned int i;
   unsigned char *pbuf;
   pbuf = databuf;
   if(Rcv_Num!=0)//接受缓冲区中有数据
   {
        pbuf[0] =PopRcv();//从收数缓冲中读取一字节数据
        if(pbuf[0] == 0xA0 && protocol == PROTOCLO_P)
        {
            int sum = 0;
                                         //校验
            for(i=1;i<6;i++)
            {
                pbuf[i] =PopRcv();//读取后面的5个字节
                sum ^= pbuf[i];
         
            }
           // PushSend(sum);
            sum ^= pbuf[0];
            pbuf[6] =PopRcv();//再读取1个字节
            sum ^= pbuf[6];
            pbuf[7] =PopRcv();//再读取1个字节：到这里一共读了8个字节
            if(pbuf[7] == sum)
            {
                //PushSend(sum);
                if(pbuf[1] == adress||pbuf[1] == 0xff||adress == 0 )
                   return 0;
                else 
                   return 1;
            }
      	    else
            {
                //PushSend(0x88);
                return 1;
            }
        }
        else if(pbuf[0] == 0xff && protocol == PROTOCLO_D)
        {
            int sum = 0;
                                         //校验
            for(i=1;i<6;i++)
            {
                pbuf[i] =PopRcv();
                sum += pbuf[i];
         
            }
            pbuf[6] =PopRcv();
            if(pbuf[6] == sum)
            {
               if(pbuf[1] == adress||pbuf[1] == 0xff||adress == 0 )
                   return 0;
                else 
                   return 1;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            //PushSend(pbuf[0]);
            return 1;                  //出错返回
        } 
   }
   else
   {
       return 1;
   }                
}


 