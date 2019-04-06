/*
* timers.c
*
* Created: 8/19/2012 11:45:45 PM
*  Author: Aaron Solochek
*/

#include "main.h"
#include "timers.h"
#include "util.h"

volatile uint8_t packetTimer;

volatile uint16_t intCnt0;
volatile uint16_t ackWaitCnt;

volatile uint32_t sysClk;
volatile uint32_t trigger;


void TimersInit()
{
    // Configure Clocks and Timers
    TIMER0->registerB.CLOCK_SELECT = TIMER_CLOCK_DIV_1;
    TIMER1->registerB.CLOCK_SELECT = TIMER_CLOCK_DIV_1;
    TIMER2->registerB.CLOCK_SELECT = TIMER_CLOCK_DIV_1;

    // Start fast clock interrupt.  31.25 kHz for internal crystal, 41.875 kHz for external 12MHz
    TIMERINTERRUPTMASK->timer0.OVERFLOW_INTERRUPT_ENABLE = TRUE;
}


#ifndef USE_INLINE_DELAY
//FIXME: this fails if us is < 128.  Theory: computing trigger is taking too long
void delay_us(uint32_t us)
{
    trigger = usToTicks(us) + sysClk;
    
    if(us >= 128) {
        triggered = 0;
        while(!triggered);
    }
}
#endif
