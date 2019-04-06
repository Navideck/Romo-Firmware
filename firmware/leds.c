/*
* leds.c
*
* Created: 10/9/2012 4:56:48 PM
*  Author: Aaron Solochek
*/


#include "main.h"
#include "leds.h"

volatile struct ledControl_t ledControl;


void LEDSOS(void)
{
    while(1) {
        ledControl.mode = RMLedModeBlink;
        LEDSetBlink(100,100);				// S
        delay_ms(500);
        ledControl.mode = RMLedModeOff;
        LEDS_OFF;
        delay_ms(300);
        ledControl.mode = RMLedModeBlink;
        LEDSetBlink(300,300);				// O
        delay_ms(1800);
        LEDSetBlink(100,100);				// S
        delay_ms(600);
        ledControl.mode = RMLedModeOff;
        LEDS_OFF;
        delay_ms(500);
    }
}