/*
* uart.h
*
* Created: 8/14/2012 1:03:39 PM
*  Author: Aaron Solochek
*/


#ifndef UART_H_
#define UART_H_

#include "romo3.h"
#include <util/setbaud.h>
#include "util.h"

#define BAUD_RATE                   57600L

#define UART1_TX_ENABLED			bit_is_set(UCSR1B, TXEN1)
#define UART1_RX_ENABLED			bit_is_set(UCSR1B, RXEN1)
#define UART1_EMPTY					bit_is_set(UCSR1A, UDRE1)
#define UART1_TX_COMPLETE			bit_is_set(UCSR1A, TXC1)
#define UART1_RX_COMPLETE			bit_is_set(UCSR1A, RXC1)

#define UART1_TX_ENABLE				sbi(UCSR1B, TXEN1);
#define UART1_TX_DISABLE			cbi(UCSR1B, TXEN1);
#define UART1_RX_ENABLE				sbi(UCSR1B, RXEN1);
#define UART1_RX_DISABLE			cbi(UCSR1B, RXEN1);

#define UART1_TRANSMIT(x)			UDR1 = x
#define UART1_TX_WHEN_READY(x)		loop_until_bit_is_set(UCSR1A, UDRE1); UDR1 = x

inline void UART1Init(void)
{
//    UBRR1 = UBRR_VALUE;

    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
    #if USE_2X
    UCSR1A |= (1 << U2X1);
    #else
    UCSR1A &= ~(1 << U2X1);
    #endif

    //#if F_CPU == 12000000 || F_CPU == 12000000L
    //UBRR1L = 0x0d;
    //cbi(UCSR1A, U2X0);
    //#else F_CPU == 8000000 || F_CPU == 8000000L
    //UBRR1L = 0x10;
    //sbi(UCSR1A, U2X0);
    //#endif
    UCSR1B = _BV(RXEN0) | _BV(TXEN0); // Enable Rx, Tx
    UCSR1C = _BV(UCSZ00) | _BV(UCSZ01); // Set character size to 8-bit
    sbi(DDRD,DDRD3); // Set pin as output
}

uint8_t UART1Receive(void);

#endif /* UART_H_ */