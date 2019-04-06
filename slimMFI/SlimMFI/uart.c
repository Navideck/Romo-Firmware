/*
* uart.c
*
* Created: 8/14/2012 1:03:51 PM
*  Author: Aaron Solochek
*/

#define UART_C

#include "uart.h"


uint8_t UART1Receive(void)
{
    loop_until_bit_is_set(UCSR1A, RXC1);
    return UDR1;
}
