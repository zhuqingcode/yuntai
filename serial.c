
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
extern unsigned char protocol;      //Э������
extern unsigned char boundrate;     //������
extern unsigned char adress;        //��ַ
void InitUart()
{
    PCON = 0;
	SCON = 0;
	SCON |= 0x40;      // 10 bit (8, N, 1)

	SCON |= 0x10;       //REN��λ���������
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
        
        *Serial_Rcv_Write = SBUF;//��SBUF�ж�ȡһ���ֽ�     
        if(Serial_Rcv_Write == (Serial_Rcvbuf + Serial_BufNum - 1))	//������BUF�Ƿ�����������ˣ��Ͱ�BUF���׵�ַ���¸�ֵ��Serial_Rcv_Write
        {															//Ҳ�������ݴ�BUF�ĵ�һ���ֽڿ�ʼ���
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

            if(Serial_Send_Read == (Serial_Sendbuf + Serial_BufNum_W - 1)) //ͬ��
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
    
    if(Serial_Send_Write == (Serial_Sendbuf + Serial_BufNum_W - 1))   // ָ������ Խ�������ָ�򻺳������ֽ�
    {
        Serial_Send_Write = Serial_Sendbuf;        
    }
    else 
    {
        Serial_Send_Write++;       // ��������ָ�� �ȴ���һ�������
    }

    Send_Num++;             // �������ݼ�������1 
    
    if(b_FlagSendOK == 0)        // ���ϴ����ݷ������ �����µķ���
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
    
    ch = *Serial_Rcv_Read;      // �����������ж�ȡһ�ֽ����� 
    *Serial_Rcv_Read=0x00;	 
     if(Serial_Rcv_Read == (Serial_Rcvbuf + Serial_BufNum - 1))
     {
         Serial_Rcv_Read = Serial_Rcvbuf;                  
     }
     else 
     {
         Serial_Rcv_Read++;     // ����ָ�� ��Խ��������ָ������׵�ַ 
     }

     Rcv_Num--;          // �������ݼ�������1 
	  //	ES = 1;
     return (ch);         // ������ֵ����
     
}


unsigned char Serial_Rcv(unsigned char *databuf)
{
   unsigned int i;
   unsigned char *pbuf;
   pbuf = databuf;
   if(Rcv_Num!=0)//���ܻ�������������
   {
        pbuf[0] =PopRcv();//�����������ж�ȡһ�ֽ�����
        if(pbuf[0] == 0xA0 && protocol == PROTOCLO_P)
        {
            int sum = 0;
                                         //У��
            for(i=1;i<6;i++)
            {
                pbuf[i] =PopRcv();//��ȡ�����5���ֽ�
                sum ^= pbuf[i];
         
            }
           // PushSend(sum);
            sum ^= pbuf[0];
            pbuf[6] =PopRcv();//�ٶ�ȡ1���ֽ�
            sum ^= pbuf[6];
            pbuf[7] =PopRcv();//�ٶ�ȡ1���ֽڣ�������һ������8���ֽ�
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
                                         //У��
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
            return 1;                  //������
        } 
   }
   else
   {
       return 1;
   }                
}


 