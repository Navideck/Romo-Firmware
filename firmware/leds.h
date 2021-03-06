﻿/*
* leds.h
*
* Created: 10/9/2012 4:56:04 PM
*  Author: Aaron Solochek
*/


#ifndef LEDS_H_
#define LEDS_H_

#include "flags.h"
#include "util.h"
#include "LEDModes.h"


register unsigned char ledPWM asm("r2");
register unsigned char ledPWMCnt asm("r3");

#define ledPulseIncreasing ((volatile devFlags1*)&GPIOR0)->ledPulseIncreasing_f



#define LEDS_ON								sbi(PORTB, PORTB0)
#define LEDS_OFF							cbi(PORTB, PORTB0)
#define LEDS_TOGGLE							sbi(PINB, PINB0)



struct ledControl_t {
    RMLedMode mode;
    uint8_t pulseStep;
    uint8_t pulseCnt;
    uint8_t pulseTrigger;
    uint16_t pulsePeriod;
    uint16_t blinkCnt;
    uint16_t blinkTriggerOn;
    uint16_t blinkTriggerOff;
};

extern volatile struct ledControl_t ledControl;


inline void LEDSetPWM(uint8_t pwm)
{
    ledControl.mode = RMLedModePWM;
    ledPWM = pwm;
}


inline void LEDSetBlink(uint16_t on, uint16_t off)
{
    ledControl.mode = RMLedModeBlink;
    ledControl.blinkCnt = 0;
    ledControl.blinkTriggerOff = msToTicks(on);
    ledControl.blinkTriggerOn = msToTicks(off) + ledControl.blinkTriggerOff;
}


inline void LEDSetBlinkWithPWM(uint16_t on, uint16_t off, uint8_t pwm)
{
    ledControl.mode = RMLedModeBlink;
    ledControl.blinkCnt = 0;
    ledControl.blinkTriggerOff = msToTicks(on);
    ledControl.blinkTriggerOn = msToTicks(off) + ledControl.blinkTriggerOff;
    ledPWM = pwm;
}


inline void LEDSetPulse(uint8_t trigger, uint8_t step)
{
    ledControl.mode = RMLedModePulse;
    ledControl.pulseTrigger = trigger;
    ledControl.pulseStep = step;
}


inline void LEDSetHalfPulseUp(uint8_t trigger, uint8_t step)
{
    ledControl.mode = RMLedModeHalfPulseUp;
    ledControl.pulseTrigger = trigger;
    ledControl.pulseStep = step;
    ledPulseIncreasing = TRUE;
}


inline void LEDSetHalfPulseDown(uint8_t trigger, uint8_t step)
{
    ledControl.mode = RMLedModeHalfPulseDown;
    ledControl.pulseTrigger = trigger;
    ledControl.pulseStep = step;
    ledPulseIncreasing = FALSE;
}


inline void LEDStep()
{
    switch(ledControl.mode) {
        case RMLedModePWM:
        case RMLedModePulse:
        case RMLedModeHalfPulseUp:
        case RMLedModeHalfPulseDown:

        //every tick we decide if LEDs should be on or off for softpwm
        (ledPWMCnt <= ledPWM) ? LEDS_ON : LEDS_OFF;

        if(ledControl.mode != RMLedModePWM) { //this will only be true here for pulse mode
            if(!ledPWMCnt) { //pulse is modified every 256 ticks
                if(ledControl.pulseCnt == ledControl.pulseTrigger) { //act every ledPulseTrigger * 256 ticks
                    if(ledPulseIncreasing) { //we are currently increasing ledPWM
                        if((255 - ledPWM) > ledControl.pulseStep) { //if we won't overflow
                            ledPWM += ledControl.pulseStep;
                        } else { //if we will overflow, set to max pwm and reverse
                            if(ledControl.mode == RMLedModeHalfPulseUp) {
                                ledPWM = (ledPWM == 255) ? 0 : 255;
                            } else {
                                ledPWM = 255;
                                ledPulseIncreasing = FALSE;
                            }
                        }
                    } else { //we are currently decreasing pwm
                        if(ledPWM > ledControl.pulseStep) { //we won't overflow
                            ledPWM -= ledControl.pulseStep;
                        } else { //we will overflow, set to min pwm and reverse
                            if(ledControl.mode == RMLedModeHalfPulseDown) {
                                ledPWM = (ledPWM == 0) ? 255 : 0;
                            } else {
                                ledPWM = 0;
                                ledPulseIncreasing = TRUE;
                            }
                        }
                    }
                    ledControl.pulseCnt = 0;
                }
                ledControl.pulseCnt++;
            }
        }
        break;
        
        case RMLedModeBlink:
        
        ledControl.blinkCnt++;
        if(ledControl.blinkCnt <= ledControl.blinkTriggerOff) {
            (ledPWMCnt <= ledPWM) ? LEDS_ON : LEDS_OFF;
        } else if(ledControl.blinkCnt <= ledControl.blinkTriggerOn) {
            LEDS_OFF;
        } else {
            ledControl.blinkCnt = 0;
        }
        break;
        
        case RMLedModeOff:
        break;
    }

    //increment the softpwm counter every tick
    ledPWMCnt++;
}

void LEDSOS(void);



#endif /* LEDS_H_ */