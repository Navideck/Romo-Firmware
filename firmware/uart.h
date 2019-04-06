/*
* uart.h
*
* Created: 8/14/2012 1:03:39 PM
*  Author: Aaron Solochek
*/


#ifndef UART_H_
#define UART_H_

#define BAUD                        57600
//#define BAUD                      230400

#define UART0_TX_ENABLED          bit_is_set(UCSR0B, TXEN0)
#define UART0_RX_ENABLED          bit_is_set(UCSR0B, RXEN0)
#define UART0_EMPTY               bit_is_set(UCSR0A, UDRE0)
#define UART0_TX_COMPLETE         bit_is_set(UCSR0A, TXC0)
#define UART0_RX_COMPLETE         bit_is_set(UCSR0A, RXC0)

#define UART0_TX_ENABLE           sbi(UCSR0B, TXEN0);
#define UART0_TX_DISABLE          cbi(UCSR0B, TXEN0);
#define UART0_RX_ENABLE           sbi(UCSR0B, RXEN0);
#define UART0_RX_DISABLE          cbi(UCSR0B, RXEN0);
#define UART0_TX_INT_ENABLE       sbi(UCSR0B, TXCIE0)
#define UART0_TX_INT_DISABLE      cbi(UCSR0B, TXCIE0)
#define UART0_RX_INT_ENABLE       sbi(UCSR0B, RXCIE0)
#define UART0_RX_INT_DISABLE      cbi(UCSR0B, RXCIE0)

#define UART0_TRANSMIT(x)         UDR0 = x
#define UART0_TX_WHEN_READY(x)    loop_until_bit_is_set(UCSR0A, UDRE0); UDR0 = x
#define UART0_TX_IF_READY(x)      if(bit_is_set(UCSRA0, UDRE0)) UDR0 = x

#define UART1_TX_ENABLED          bit_is_set(UCSR1B, TXEN1)
#define UART1_RX_ENABLED          bit_is_set(UCSR1B, RXEN1)
#define UART1_EMPTY               bit_is_set(UCSR1A, UDRE1)
#define UART1_TX_COMPLETE         bit_is_set(UCSR1A, TXC1)
#define UART1_RX_COMPLETE         bit_is_set(UCSR1A, RXC1)

#define UART1_TX_ENABLE           sbi(UCSR1B, TXEN1);
#define UART1_TX_DISABLE          cbi(UCSR1B, TXEN1);
#define UART1_RX_ENABLE           sbi(UCSR1B, RXEN1);
#define UART1_RX_DISABLE          cbi(UCSR1B, RXEN1);
#define UART1_TX_INT_ENABLE       sbi(UCSR1B, TXCIE1)
#define UART1_TX_INT_DISABLE      cbi(UCSR1B, TXCIE1)
#define UART1_RX_INT_ENABLE       sbi(UCSR1B, RXCIE1)
#define UART1_RX_INT_DISABLE      cbi(UCSR1B, RXCIE1)

#define UART1_TRANSMIT(x)         UDR1 = x
#define UART1_TX_WHEN_READY(x)    loop_until_bit_is_set(UCSR1A, UDRE1); UDR1 = x
#define UART1_TX_IF_READY(x)      if(bit_is_set(UCSRA1, UDRE1)) UDR1 = x

void UART0Init(void);
uint8_t UART0Receive(void);
void UART0TransmitCString(volatile char *s);
void UART0TransmitUInt8(uint8_t number, uint8_t zeroPadding);
void UART0TransmitUInt16(uint16_t number, uint8_t zeroPadding);

void UART1Init(void);
uint8_t UART1Receive(void);
void UART1TransmitCString(volatile char *s);
void UART1TransmitUInt8(uint8_t number, uint8_t zeroPadding);
void UART1TransmitUInt16(uint16_t number, uint8_t zeroPadding);



#endif /* UART_H_ */