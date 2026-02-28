#ifndef UART_H
#define UART_H

/* UART Buffer Defines */ 
#define UART_RX_BUFFER_SIZE 4     /* 2,4,8,16,32,64,128 or 256 bytes */ 
#define UART_TX_BUFFER_SIZE 4 

void UART_Open(void);
void UART_DisableReciveIRQ(void);
void UART_EnableReciveIRQ(void);
unsigned char UART_Receive(void);
void UART_Transmit(unsigned char tx_data);
void UART_Sendstr(char *strbuf);
unsigned char UART_DataInReceiveBuffer(void);

#endif