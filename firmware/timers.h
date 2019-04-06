/*
* timers.h
*
* Created: 8/19/2012 11:46:11 PM
*  Author: Aaron Solochek
*/


#ifndef TIMERS_H_
#define TIMERS_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include "registers.h"
#include "util.h"
#include "time_utils.h"

#define CHECK_TRIGGER			(triggered ? TRUE : FALSE)

extern volatile uint8_t packetTimer;
extern volatile uint16_t ackWaitCnt;
extern volatile uint32_t sysClk;
extern volatile uint32_t trigger;


register unsigned char intCnt1 asm("r6");
register unsigned char loopTrigger asm("r7");


#define triggered ((volatile devFlags1*)&GPIOR0)->waitTriggered_f
#define loopTriggered ((volatile devFlags1*)&GPIOR0)->loopTrigger_f


#ifdef USE_INLINE_DELAY
inline void delay_us(uint32_t us)
{
    trigger = usToTicks(us) + sysClk;
    triggered = 0;
    while(!triggered);
}
#else
void delay_us(uint32_t us);
#endif

inline void delay_ms(uint32_t ms)
{
    delay_us(1000*ms);
}


inline void TimerStep(void)
{
    sysClk++;
    intCnt0++;
    intCnt1++;
    ackWaitCnt++;
    loopTrigger++;
    packetTimer++;

    if(sysClk == trigger) {
        triggered = 1;
    }
    if(loopTrigger == 8) {
        loopTriggered = 1;
    }
}

void TimersInit(void);



#endif /* TIMERS_H_ */