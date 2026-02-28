#include <at89x051.h>
#include "uart.h"

#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1 ) 
#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK ) 
   #error RX buffer size is not a power of 2 
#endif 

#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1 ) 
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK ) 
   #error TX buffer size is not a power of 2 
#endif 

//Flags 
#define UART_FLAG_TX_IN_PROGRESS   	0x00 
#define UART_FLAG_TX_COMPLETE      	0x01 
#define UART_FLAG_ERR         		0x02 

volatile struct uart0_data { 
   unsigned char rx_buf[UART_RX_BUFFER_SIZE]; 
   unsigned char rx_head; 
   unsigned char rx_tail; 
    
   unsigned char tx_buf[UART_TX_BUFFER_SIZE]; 
   unsigned char tx_head; 
   unsigned char tx_tail; 
   unsigned char flags; 
}uart; 


//:::::::::::::::::::::::::::::::::::::::::::::::::::::: 
// Init UART0 
//:::::::::::::::::::::::::::::::::::::::::::::::::::::: 
void UART_Open(void) { 
  SCON = 0x50;                 		//SCON: mode 1, 8-bit UART, enable rcvr 
  TMOD |= 0x20;                  	//TMOD: timer 1, mode 2, 8-bit reload 
  TH1   = 0xFD;                     //TH1:  reload value for 9600 x 11.0592 
  TR1 = 1;                        	//TR1:  timer 1 run              
  ES   = 1;                  		//Разрешить прерывание от UART 
} 


void UART_DisableReciveIRQ(void) { 
   REN = 0; 
} 


void UART_EnableReciveIRQ(void) { 
   RI = 0; 
   REN = 1; 
} 


unsigned char UART_Receive(void) { 
     unsigned char tmptail; 
  
     tmptail =  uart.rx_tail; 
     while (uart.rx_head == tmptail); 
     tmptail = (uart.rx_tail + 1) & UART_RX_BUFFER_MASK; 
     uart.rx_tail = tmptail;    

     return uart.rx_buf[tmptail]; 
} 


void UART_Transmit(unsigned char tx_data) { 
   unsigned char tmphead; 

     tmphead = (uart.tx_head + 1) & UART_TX_BUFFER_MASK; 
     while (tmphead == uart.tx_tail); 

     uart.tx_buf[tmphead] = tx_data;          
     uart.tx_head = tmphead;  
    
     if((uart.flags & (1<<UART_FLAG_TX_IN_PROGRESS)) == 0) { 
      TI = 1; 
      uart.flags |= (1<<UART_FLAG_TX_IN_PROGRESS); 
      uart.flags &= ~(1<<UART_FLAG_TX_COMPLETE); 
   } 
} 


void UART_Sendstr(char *strbuf) { 
     while (*strbuf != 0) UART_Transmit(*strbuf++); 
} 

unsigned char UART_DataInReceiveBuffer(void) { 
     unsigned char tmptail; 
      
     tmptail =  uart.rx_tail; 
     return (uart.rx_head != tmptail); 
} 


void uart_isr (void) __interrupt (4) __using (2) { 
   volatile unsigned char tmphead; 
   volatile unsigned char tmptail; 

   if(RI) { 
      tmphead = (uart.rx_head + 1) & UART_RX_BUFFER_MASK; 
      uart.rx_head = tmphead; 
      if(tmphead == uart.rx_tail) { 
         //ERROR! Receive buffer overflow 
         uart.flags |= (1<<UART_FLAG_ERR); 
      } 
      uart.rx_buf[tmphead] = SBUF; 

      RI = 0; 
    } 
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 

   if(TI) { 
      tmptail = uart.tx_tail; 
      if(uart.tx_head != tmptail) { 
         tmptail = (uart.tx_tail + 1) &   UART_TX_BUFFER_MASK; 
         uart.tx_tail = tmptail; 
         SBUF = uart.tx_buf[tmptail]; 
      } 
         else { 
               uart.flags &= ~(1<<UART_FLAG_TX_IN_PROGRESS);    
               uart.flags |= (1<<UART_FLAG_TX_COMPLETE); 
            } 
      TI = 0; 
   } 
} 