/*
* uart.c
*
* Created: 8/14/2012 1:03:51 PM
*  Author: Aaron Solochek
*/

#define UART_C

#include "main.h"
#include "uart.h"
#include "ringbuffer.h"
#include "util.h"
#include "registers.h"
#include <util/setbaud.h>


void UART0Init(void)
{
    // Initialize as much of the UART0 as we can.
    UART0->baud = UBRR_VALUE;
    UART0->registerA.SPEED_DOUBLE = TRUE;
    UART0->registerB.RECEIVER_ENABLE = TRUE;
    UART0->registerB.TRANSMITTER_ENABLE = TRUE;
    #ifdef USE_UART0_RECEIVE_INTERRUPT
    UART0->registerB.RECEIVE_COMPLETE_INTERRUPT_ENABLE = TRUE;
    #endif
    GPIO->portD.direction.P1 = OUTPUT;
}

uint8_t UART0Receive(void)
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void UART0TransmitCString(volatile char *s)
{
    int i = 0;
    do {
        UART0_TX_WHEN_READY(s[i++]);
    } while (s[i] != '\0');
}


void UART0TransmitUInt8(uint8_t number, uint8_t zeroPadding)
{
    char val[2];
    
    Int8toAscii(number, val);

    if(zeroPadding || (val[0] != 0x30)) {
        UART0_TX_WHEN_READY(val[0]);
    }
    
    UART1_TX_WHEN_READY(val[1]);
}


void UART0TransmitUInt16(uint16_t number, uint8_t zeroPadding)
{
    char val[4];
    uint8_t nonZero = FALSE;
    
    Int16toAscii(number, val);
    
    for(uint8_t i=0; i<4; i++) {
        if(val[i] != 0x30) {
            nonZero = TRUE;
        }
        if(zeroPadding || nonZero || (i == 3)) {
            UART0_TX_WHEN_READY(val[i]);
        }
    }
}

void UART1Init(void)
{
    // Initialize as much of the UART1 as we can.
    UART1->baud = UBRR_VALUE;
    UART1->registerA.SPEED_DOUBLE = USE_2X;
    UART1->registerB.RECEIVER_ENABLE = TRUE;
    UART1->registerB.TRANSMITTER_ENABLE = TRUE;
    #ifdef USE_UART1_RECEIVE_INTERRUPT
    UART1->registerB.RECEIVE_COMPLETE_INTERRUPT_ENABLE = TRUE;
    #endif
    GPIO->portD.direction.P3 = OUTPUT;
}

uint8_t UART1Receive(void)
{
    loop_until_bit_is_set(UCSR1A, RXC1);
    return UDR1;
}

void UART1TransmitCString(volatile char *s)
{
    int i = 0;
    do {
        UART1_TX_WHEN_READY(s[i++]);
    } while (s[i] != '\0');
}


void UART1TransmitUInt8(uint8_t number, uint8_t zeroPadding)
{
    char val[2];
    
    Int8toAscii(number, val);

    if(zeroPadding || (val[0] != 0x30)) {
       UART1_TX_WHEN_READY(val[0]);
    }
    
    UART1_TX_WHEN_READY(val[1]);
}


void UART1TransmitUInt16(uint16_t number, uint8_t zeroPadding)
{
    char val[4];
    uint8_t nonZero = FALSE;
    
    Int16toAscii(number, val);
    
    for(uint8_t i=0; i<4; i++) {
        if(val[i] != 0x30) {
            nonZero = TRUE;
        }
        if(zeroPadding || nonZero || (i == 3)) {
            UART1_TX_WHEN_READY(val[i]);
        }
    }
}

#ifdef USE_UART1_RECEIVE_INTERRUPT
ISR(USART1_RX_vect, ISR_BLOCK)
{
    if(BytesFreeInBuf() && MutexTryLockInBuf()) {
        PutByteInBuf(UDR1);
        MutexUnlockInBuf();
    }
}
#endif