/*
* dance.c
*
* Created: 8/13/2012 8:24:31 PM
*  Author: Aaron Solochek
*/


#include "main.h"
#include "dance.h"
#include "pwm.h"
#include "util.h"
#include "timers.h"
#include "leds.h"
#include "eeprom.h"


void DanceMain()
{
    uint16_t samples[30];
    int i = 0;
    
    LEDS_ON;
    delay_ms(3000);
    LEDS_OFF;
    ADC_SET_CHAN(ADC_M3CURRENT);
    TILT_BACKWARD(255);		// Tilt backward
    
    ADC_START_CONVERSION;
    delay_ms(250);
    samples[i++] = ANALOGIN->data;
    ADC_START_CONVERSION;
    delay_ms(250);
    samples[i++] = ANALOGIN->data;
    ADC_START_CONVERSION;
    delay_ms(250);
    samples[i++] = ANALOGIN->data;
    delay_ms(250);
    
    TILT_STOP();		    // Stop tilt
    delay_ms(200);
    TILT_FORWARD(255);		// Tilt forward
    
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    delay_ms(2000);
    
    TILT_STOP();		    // Stop tilt
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    delay_ms(1000);
    FORWARD(255);           // Go Forward for 3.5 seconds
    
    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    delay_ms(500);

    STOP();                 // Stop
    delay_ms(500);
    
    BACKWARD(255);          // Go Backwards
    
    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;

    TILT_BACKWARD(255);

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(500);
    samples[i++] = ANALOGIN->data;

    STOP();
    delay_ms(500);
    TURN_RIGHT(255);        // Turn Right
    
    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    delay_ms(150);

    STOP();                 // Stop
    delay_ms(200);
    TURN_LEFT(255);         // Turn Left

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;

    ADC_SET_CHAN(ADC_M1CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    ADC_SET_CHAN(ADC_M2CURRENT);
    ADC_START_CONVERSION;
    delay_ms(75);
    samples[i++] = ANALOGIN->data;
    delay_ms(150);

    STOP();

    eeprom_write_block((void*)samples, EEPROM_DANCE_MODE_DATA_ADDRESS, 60);
    delay_ms(500);
    
    STOP();                 // Stop
    TILT_STOP();

    while(1) {
        LEDS_ON;
        delay_ms(500);
        LEDS_OFF;
        delay_ms(500);
    }
    
}