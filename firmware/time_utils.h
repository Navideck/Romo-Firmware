/*
 * time_utils.h
 *
 * Created: 3/7/2013 10:40:34 AM
 *  Author: Aaron Solochek
 */ 


#ifndef TIME_UTILS_H_
#define TIME_UTILS_H_

#include "util.h"

// ticks of the timer interrupt
#define __US_TO_TICKS(us)       ((us/256)*(F_CPU/1000000L))

// ticks of the timer1 interrupt
#define __US_TO_CNT1(us)        (us / (1024/(F_CPU/1000000)))

// These macros assume you are using intCnt0 for timing, and will cause
// a compile error if the time specified is out of range
#define US_TO_TICKS(us)         (U_BOUNDS_CHECK(__US_TO_TICKS(us), intCnt0))
#define MS_TO_TICKS(ms)         US_TO_TICKS(ms*1000L)

// These macros assume you are using intCnt1 for timing, and will cause
// a compile error if the time specified is out of range
#define US_TO_CNT1(us)         (U_BOUNDS_CHECK(__US_TO_CNT1(us), TCNT1))
#define MS_TO_CNT1(ms)         US_TO_CNT1(ms*1000L)


//?? We could assign some of these to registers for speed
extern volatile uint16_t intCnt0;


/// return the number of fast interrupt ticks that occur in the given
/// number of microseconds.  For CPU clocks that aren't a multiple of 8MHz,
/// this is a close approximation.
/// For static values, use the US_TO_TICKS macro
inline uint32_t usToTicks(uint32_t us)
{
    #if F_CPU == 12000000 || F_CPU == 12000000L || F_CPU == 12000000UL
    return (us >> 5) + (us >> 6) + (us >> 11) + (us >> 12);
    #elif F_CPU == 8000000 || F_CPU == 8000000L || F_CPU == 8000000UL
    return (us >> 5);
    #elif F_CPU == 16000000 || F_CPU == 16000000L || F_CPU == 16000000UL
    return (us >> 4);
    #endif
}


inline uint32_t msToTicks(uint32_t ms)
{
    return usToTicks(1000*ms);
}


#endif /* TIME_UTILS_H_ */