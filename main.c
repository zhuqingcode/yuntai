#include <AT89X52.h>
//#include <reg52.h>
#include "serial.h"
#include "i2c.h"
static unsigned int count;  //计数
static unsigned int pcount=248;  //水平计数
static unsigned int fcount=48;  //垂直计数
static int step_index,up_index;  //步进索引数，值为0－7
extern unsigned int Rcv_Num;
/*朱庆加在这儿的，为了解决陈云峰提出的问题@2012-9-26*/
unsigned char break_flag = 0;
unsigned char idata rcvbuf[8]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
/****************************************************/
unsigned char protocol = 0;      //协议类型
unsigned char boundrate = 0;     //波特率
unsigned char adress = 0;        //地址
static bit turn,up_turn;  //步进电机转动方向
static bit stop_flag,up_flag;  //步进电机停止标志
static int speedlevel,up_speed; //步进电机转速参数，数值越大速度越慢，最小值为1，速度最快
static int spcount,up_spcount;    //步进电机转速参数计数
void delay(unsigned int endcount);  //延时函数，延时为endcount*0.5毫秒
void gorun();          //步进电机控制步进函数
void up_downrun();

/*
 *初始化地址，协议，波特率等参数
 *
 */
void param_init()
{
   //adress = P0_0<<7|P0_1<<6|P0_2<<5|P0_3<<4|P0_4<<3|P0_5<<2|P0_6<<1|P0_7;
   if(P0_0 == 1)
     adress |= 0x1<<7;
   if(P0_1 == 1)
     adress |= 0x1<<6;
   if(P0_2 == 1)
     adress |= 0x1<<5;
   if(P0_3 == 1)
     adress |= 0x1<<4;
   if(P0_4 == 1)
     adress |= 0x1<<3;
   if(P0_5 == 1)
     adress |= 0x1<<2;
   if(P0_6 == 1)
     adress |= 0x1<<1;
   if(P0_7 == 1)
     adress |= 0x1;
   adress ^= 0xff;
   //adress -= 1;	  //Zhuqing add it here 2011.10.11
   if( P1_0 == 1 && P1_1 == 0)
   {
     protocol = PROTOCLO_D;
   }
   else
   {
     protocol = PROTOCLO_P;
   }
   //protocol = PROTOCLO_D;
   if(P1_2 == 0 && P1_3 == 1)
   {
     boundrate = BOUNDRATE_4800;
   }
   else
   {
     boundrate = BOUNDRATE_9600; 
   }
   //boundrate = BOUNDRATE_9600; 
}
void port_init()
{
    P1_4 = 0;
    P1_5 = 0;
    P1_6 = 0;
    P1_7 = 0;
    P3_4 = 0;
    P3_5 = 0;
    P3_6 = 0;
    P3_7 = 0;

}

/*void timer0_start()
{
   TR0 =1;
}


void timer0_stop()
{
   TR0 =0;
}*/

void timer0_init()
{
    EA = 1;             //允许CPU中断 
    TMOD = 0x11; //设定时器0和1为16位模式1 
    ET0 = 1;           //定时器0中断允许 
   // TH0 = 0xFC;
   // TL0 = 0x18;   //设定时每隔0.5ms中断一次  0xFC   0x18
   // TH0 = 0xF7;
   // TL0 = 0xE0;   //设定时每隔1ms中断一次  0xF7   0xE0
    TH0 = 0xFD;
    TL0 = 0xF7;   //设定时每隔0.25ms中断一次  0xF7   0xE0
    TR0 = 1;         //开始计数

}


 
//定时器0中断处理 
void timeint(void) interrupt 1 
{
   
  TH0=0xFD;
  TL0=0xF7; //设定时每隔0.25ms中断一次
  count++;
  
  spcount--;
  up_spcount--;
  if(spcount<=0)
  {
    spcount = speedlevel;
    gorun();
  }
  if(up_spcount<=0)
  {
    up_spcount = up_speed;
    up_downrun();
  }

}

void delay(unsigned int endcount)
{
  count=0;
  do{}while(count<endcount);
}

/*
 *控制上下转电机
 *
 */
void up_downrun()
{
  if (up_flag==1)
  {
    P3_4 = 0;
    P3_5 = 0;
    P3_6 = 0;
    P3_7 = 0;
    //timer0_stop();
    return;
  }

  switch(up_index)
  {
  case 0: //0
    P3_4 = 1;
    P3_5 = 0;
    P3_6 = 0;
    P3_7 = 0;
    break;
  case 1: //0、1
    P3_4 = 1;
    P3_5 = 1;
    P3_6 = 0;
    P3_7 = 0;
    break;
  case 2: //1
    P3_4 = 0;
    P3_5 = 1;
    P3_6 = 0;
    P3_7 = 0;
    break;
  case 3: //1、2
    P3_4 = 0;
    P3_5 = 1;
    P3_6 = 1;
    P3_7 = 0;
    break;
  case 4:  //2
    P3_4 = 0;
    P3_5 = 0;
    P3_6 = 1;
    P3_7 = 0;
    break;
  case 5: //2、3
    P3_4 = 0;
    P3_5 = 0;
    P3_6 = 1;
    P3_7 = 1;
    break;
  case 6: //3
    P3_4 = 0;
    P3_5 = 0;
    P3_6 = 0;
    P3_7 = 1;
    break;
  case 7: //3、0
    P3_4 = 1;
    P3_5 = 0;
    P3_6 = 0;
    P3_7 = 1;
  }

  if (up_turn==1)
  {
    up_index++;
    if (up_index>7)
    {
      up_index=0;
    }
    fcount++;
  }
  else
  {
    up_index--;
    if (up_index<0)
    {
      up_index=7;
      
    }
    fcount--;
  }
    
}


 /*
 *控制左右转电机
 *
 */
void gorun()
{
  if (stop_flag==1)
  {
    P1_4 = 0;
    P1_5 = 0;
    P1_6 = 0;
    P1_7 = 0;
    //timer0_stop();
    return;
  }

  switch(step_index)
  {
  case 0: //0
    P1_4 = 1;
    P1_5 = 0;
    P1_6 = 0;
    P1_7 = 0;
    break;
  case 1: //0、1
    P1_4 = 1;
    P1_5 = 1;
    P1_6 = 0;
    P1_7 = 0;
    break;
  case 2: //1
    P1_4 = 0;
    P1_5 = 1;
    P1_6 = 0;
    P1_7 = 0;
    break;
  case 3: //1、2
    P1_4 = 0;
    P1_5 = 1;
    P1_6 = 1;
    P1_7 = 0;
    break;
  case 4:  //2
    P1_4 = 0;
    P1_5 = 0;
    P1_6 = 1;
    P1_7 = 0;
    break;
  case 5: //2、3
    P1_4 = 0;
    P1_5 = 0;
    P1_6 = 1;
    P1_7 = 1;
    break;
  case 6: //3
    P1_4 = 0;
    P1_5 = 0;
    P1_6 = 0;
    P1_7 = 1;
    break;
  case 7: //3、0
    P1_4 = 1;
    P1_5 = 0;
    P1_6 = 0;
    P1_7 = 1;
  }

  if (turn==0)
  {
    step_index++;
    if (step_index>7)
      step_index=0;
    pcount--;
  }
  else
  {
    step_index--;
    if (step_index<0)
      step_index=7;
    pcount++;
  }
    
}



 /*
 ** 扫描  left 左边界，right 右边界，col垂直位置
 ** left right 都为0的时候自动巡航
 **/
void Scan(unsigned int left,unsigned int right,unsigned int col)
{
    unsigned char tmp;//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
	stop_flag = 1;
    up_flag = 1;
    if(fcount>col)
       up_turn = 0;
    else
       up_turn = 1;

    if(left == right)
    {
       
       
       stop_flag = 0;
       delay(1000);
       up_flag = 0;
       while(1)
       {
           if((fcount >= col)&&(up_turn == 1)||(fcount <= col)&&(up_turn == 0))
           {
              up_flag=1;
           }
           if(P2_0 == 0||P2_1 == 0)
           {
               stop_flag = 1;
               //delay(1000);
               turn ^= 1;
               stop_flag = 0;
               delay(2000);
           }
           
           if(Rcv_Num>7) /*原来的判断语句*/
           {
             tmp = Serial_Rcv(rcvbuf);	//读取8个字节 朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 if(tmp == 0)			   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 {						   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 	stop_flag = 1;
            	up_flag=1;
				break_flag = 1;			//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
				break;
			 }							//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
           }
       } 
    }
    else
    {
       if(pcount>=left)
         turn=0;
       else if(pcount<=right)
         turn=1;
       else
       {
       }
       
       
       stop_flag = 0;
       delay(1000);
       up_flag = 0;
       while(1)
       {
           if((fcount >= col)&&(up_turn == 1)||(fcount <= col)&&(up_turn == 0))
           {
              up_flag=1;
           }
           if((pcount >= left&&turn == 1)||(pcount <= right&&turn == 0))
           {
               stop_flag = 1;
               //delay(1000);
               turn ^= 1;
               stop_flag = 0;
               delay(1000);
           }
           
           if(Rcv_Num>7) /*原来的判断语句*/
           {
             tmp = Serial_Rcv(rcvbuf);	//读取8个字节 朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 if(tmp == 0)			   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 {						   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 	stop_flag = 1;
            	up_flag=1;
				break_flag = 1;			//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
				break;
			 }							//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
           }
       } 
    }
}

 /*
 ** 转至预置位  row 水平位置，col 垂直位置
 ** 
 **/
void Goto_Reset(unsigned int row,unsigned int col)
{
    unsigned char tmp;//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
	if(pcount>row)
       turn = 0;
    else 
       turn = 1;
    if(fcount>col)
       up_turn = 0;
    else
       up_turn = 1;
     stop_flag = 0;
     up_flag = 0;
     while(!stop_flag||!up_flag)
     {
          if(pcount == row)
              stop_flag = 1;
          if(fcount == col)
              up_flag = 1;
          if(Rcv_Num>7) /*原来的判断语句*/
           {
             tmp = Serial_Rcv(rcvbuf);	//读取8个字节 朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 if(tmp == 0)			   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 {						   	//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
			 	stop_flag = 1;
            	up_flag=1;
				break_flag = 1;			//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
				break;
			 }							//朱庆加载这儿，为了解决陈云峰提出的问题@2012-9-27
           } 
     }
}

 /*
 ** 区间扫描
 ** 
 **/
void Qu_Scan()
{
   unsigned char tmp;
   unsigned int leftrow,rightrow,col,temp;
   //Set_addr(2000,LEFT_ADDR);
  // Set_addr(1000,RIGHT_ADDR);
   tmp = Read_addr(&col,QCOL_ADDR);
   tmp = Read_addr(&leftrow,LEFT_ADDR);
   tmp = Read_addr(&rightrow,RIGHT_ADDR);
   
   
   if(tmp == 0)
   {
     if(leftrow>20000||leftrow<248)
     {
      leftrow = 20000;
     }
     if(rightrow>20000||rightrow<248)
     {
      rightrow = 248;
     }
     Set_State(QU_STATE,STATE_ADDR);
     if(leftrow < rightrow)
     {
       temp = leftrow;
       leftrow = rightrow;
       rightrow = temp;
     }
     if(col > 2200 || col<248 )
     {
      col = 248;
     }
     Scan(leftrow,rightrow,col);
     Set_State(0,STATE_ADDR);
     delay(1000);
   }
   
}

 /*
 ** 自动巡航
 ** 
 **/
void Auto_Scan()
{
    unsigned char tmp;
    unsigned int col;
    tmp = Read_addr(&col,SCAN_ADDR);
    if(col > 2200 || col<248 )
    {
      col = 248;
    }
    Set_State(AUTO_STATE,STATE_ADDR);
    if(tmp == 0)
    {
       Scan(0,0,col);
    }
    else
    {
       Scan(0,0,248);
    }
    Set_State(0,STATE_ADDR);
     delay(1000);
}


 /*
 ** 初始化
 ** 
 **/
void go_init()
{
   unsigned int dcount=0;
   unsigned char tmp,state=0x0;
   dcount = 40820;               //转180度所走的步数*4 
   while(dcount)
   {
     if(P2_0 == 0)
     {
       stop_flag = 1;
       turn ^=1;
       speedlevel = 4;
       pcount = 248;
       stop_flag = 0;
       delay(dcount);
       stop_flag = 1;
       dcount=0;
       
     }
     if(P3_2 == 0)
     {
        up_flag = 1;
        fcount = 248;
     }
   }
   delay(1000);
   tmp = Read_State(&state,STATE_ADDR);
   if(tmp == 0)
   {
      switch(state)
      {
         case QU_STATE:
           Qu_Scan();
         break;
         case AUTO_STATE:
           Auto_Scan();
         break;
         case OTEHER_STATE:
         break;
         default:
         break;
      }
   }
  

}

 /*
 ** 复位
 ** 
 **/
void go_home()
{
   unsigned int dcount=0;
   dcount = 40820;               //转180度所走的步数*4 
   stop_flag =1;
   up_flag = 1;
   turn = 0;
   up_turn = 0;
   speedlevel = 4;
   up_speed = 20;
   stop_flag = 0;
   up_flag = 0;
   while(dcount)
   {
     if(P2_0 == 0)
     {
       stop_flag = 1;
       turn ^=1;
       speedlevel = 4;
       pcount = 248;
       stop_flag = 0;
       delay(dcount);
       stop_flag = 1;
       dcount=0;
       
     }
     if(P3_2 == 0)
     {
        up_flag = 1;
        fcount = 248;
     }
   }
}


int main()
{
    unsigned char idata tmp=0,i=0;
    unsigned int  col=0;
    unsigned int  row=0;
    //unsigned char idata rcvbuf[8]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    count = 0;
    step_index = 0;
    up_index = 0;
    spcount = 0;
    up_spcount = 0;
    turn = 0;
    up_turn = 0;
    param_init();//初始化地址，协议，波特率等参数
    port_init();
    timer0_init();
    InitUart();
    speedlevel = 4;
    up_speed = 20;
    //delay(1000);
    stop_flag = 0;
    up_flag = 0;
    go_init();
    do
    {
      
      if(P2_0 == 0||P2_1 == 0)
      {
        stop_flag = 1;
      }
      if(P3_2 == 0 || fcount >= 2200)
      {
        up_flag = 1;
        if(fcount >= 2200)
        {
           fcount =2200;
        }
        else
           fcount =248;
      }
      //PushSend(0x44);
	  if(break_flag == 0)
	  {
	  	tmp = Serial_Rcv(rcvbuf);	//读取8个字节
	  }
	  else if(break_flag == 1)
	  {
	  	break_flag = 0;
		tmp = 0;
	  }
      //tmp = Serial_Rcv(rcvbuf);	//读取8个字节
      
      if(tmp == 0)
      {
        
		switch (rcvbuf[3])
        {
           case 0x02:                       //右
              if(P2_0 != 0)
              {
                stop_flag = 1;
                delay(1000);
                turn = 0;
                if(rcvbuf[4]>=0x3f)
                   speedlevel = 4;
                else
                   speedlevel = 4+0x3f-rcvbuf[4];
					spcount = speedlevel;
                stop_flag = 0;
			  }
              break;
           case 0x04:                       //左
              if(P2_1 != 0)
              {
                stop_flag = 1;
                delay(1000);
                turn = 1;
                if(rcvbuf[4]>=0x3f)
                   speedlevel = 4;
                else
                   speedlevel = 4+0x3f-rcvbuf[4];
					spcount = speedlevel;
                stop_flag = 0;
			  }
              break;
           case 0x08:                       //上
              if(fcount < 2200)
              {
                up_flag = 1;
                up_turn = 1;
                if(rcvbuf[5]>=0x3f)
                   up_speed = 16;
                else
                   up_speed = 16+0x3f-rcvbuf[5];
					up_spcount = up_speed;
                up_flag = 0;
                if(P3_2 == 0)
                   delay(3000);
              }
              break;
           case 0x10:                       //下
              if(P3_2 != 0)
              {
                up_flag = 1;
                up_turn = 0;
                if(rcvbuf[5]>=0x3f)
                   up_speed = 16;
                else
                   up_speed = 16+0x3f-rcvbuf[5];
					up_spcount = up_speed;
                up_flag = 0;
              }
              break;
           case 0x03:                       //设置预置位
              Set_Preset(pcount,fcount,rcvbuf[5]);
              break;
           case 0x07:
              {
                switch (rcvbuf[5])
                {
                  case 0x5c:                       //设置区间扫描的左位置
                   // Clear_addr(LEFT_ADDR) ;
                    Set_addr(pcount,LEFT_ADDR);
                    //PushSend(0x66);
                    break;
                  case 0x5d:                       //设置区间扫描的右位置
                    //Clear_addr(RIGHT_ADDR) ;
                    Set_addr(pcount,RIGHT_ADDR);
                    break;
                  case 0x50:                       //设置区间扫描的垂直位置
                    Set_addr(fcount,QCOL_ADDR);
                    break;
                  case 0x61:                       //启动区间扫描
                    Qu_Scan();
                   break;
                  case 0x4f:                       //设置scan的垂直位置
                    Set_addr(fcount,SCAN_ADDR);
                    break;
                  case 0x63:                       //自动巡航
					Auto_Scan();
                    break;
                  case 0x60:                       //回到中点
                    //Goto_Reset(10405,248);
                    go_home();
                    break;
                  default:
                    if(rcvbuf[5]<=0x20)
                    {                   //转至预置位
                      stop_flag = 1;
                      up_flag = 1;
                      tmp = Read_Preset(&row,&col,rcvbuf[5]);
                      if(tmp == 0)
                      {
                        Goto_Reset(row,col);
                      }
                      else
                      {
                        //PushSend(0x11);
                      } 
                    }
                    break;
                }
              }
              break;
           
           case 0x00:                       //停止
              stop_flag = 1;
              up_flag = 1;
              break;
           case 0x05:                       //删除预置位
              Clear_Preset(rcvbuf[5]);
              break;

        }
       }
       //for(i=0;i<8;i++)//朱庆注释了这儿
       //  rcvbuf[i]=0x0;
      delay(1000); 
     
      
    }while(1);
    //PushSend(0xbb);
}


