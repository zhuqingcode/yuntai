#ifndef SERIAL_H

#define SERIAL_H
#include <AT89X52.h>


#define Serial_BufNum	24
#define Serial_BufNum_W	8


#define BOUNDRATE_9600 0
#define BOUNDRATE_4800 1
#define PROTOCLO_P     0
#define PROTOCLO_D     1

void InitUart();
void  PushSend (unsigned char byte);
unsigned char PopRcv   (void);
void PrintStr(unsigned char *ptr);
unsigned char Serial_Rcv(unsigned char *databuf);

#endif   /*UART_H*/