/*
* pwm.c
*
* Created: 8/13/2012 8:15:30 PM
*  Author: Aaron Solochek
*/

#include "main.h"
#include "pwm.h"
#include "util.h"
#include "registers.h"


/**
* Initialize PWM: Timer0, Timer1, and Timer2 in PWM mode.
*/
void PWMInit(void)
{
    GPIO->portB.direction.P3 = OUTPUT;
    GPIO->portB.direction.P4 = OUTPUT;
    GPIO->portD.direction.P4 = OUTPUT;
    GPIO->portD.direction.P5 = OUTPUT;
    GPIO->portD.direction.P6 = OUTPUT;
    GPIO->portD.direction.P7 = OUTPUT;
    

    // Setup PWM0
    TIMER0->registerA.WAVEFORM_GENERATION_MODE_L = WAVEGEN_PWM_FAST;

    // Setup PWM1
    TIMER1->registerA.WAVEFORM_GENERATION_MODE_L = (WAVEGEN_PWM_FAST_8_BIT & 0x03);

    // Setup PWM2
    TIMER2->registerA.WAVEFORM_GENERATION_MODE_L = WAVEGEN_PWM_FAST;
}


void SetPWM(PWM_CHANNEL chan, int16_t val) {
    switch(chan) {
        case PWM0:						// Right motor
        if(val >= 0) {
            sbi(TCCR0A, COM0A1);		// Set PWM on output A
            cbi(TCCR0A, COM0B1);		// Disable PWM on output B
            OCR0A = (uint8_t) val;
            OCR0B = 0;
            cbi(PORTB, PORTB4);			// Set 0 on output B
        } else {
            cbi(TCCR0A, COM0A1);
            sbi(TCCR0A, COM0B1);
            OCR0A = 0;
            cbi(PORTB, PORTB3);			// Set 0 on output A
            OCR0B = (uint8_t) -val;
        }
        break;
        case PWM1:
        if(val >= 0) {
            sbi(TCCR1A, COM1A1);
            cbi(TCCR1A, COM1B1);
            OCR1AH = 0;
            OCR1AL = (uint8_t) val;
            OCR1BH = 0;
            OCR1BL = 0;
        } else {
            cbi(TCCR1A, COM1A1);
            sbi(TCCR1A, COM1B1);
            OCR1AH = 0;
            OCR1AL = 0;
            OCR1BH = 0;
            OCR1BL = (uint8_t) (-val);
        }
        break;
        case PWM2:
        if(val >= 0) {
            sbi(TCCR2A, COM2A1);
            cbi(TCCR2A, COM2B1);
            OCR2A = (uint8_t) val;
            OCR2B = 0;
        } else {
            cbi(TCCR2A, COM2A1);
            sbi(TCCR2A, COM2B1);
            OCR2A = 0;
            OCR2B = (uint8_t) -val;
            break;
        }

    }
}
