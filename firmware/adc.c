/*
* adc.c
*
* Created: 8/13/2012 8:23:01 PM
*  Author: Aaron Solochek
*/

#include "main.h"
#include "adc.h"
#include "util.h"
#include "registers.h"
#include <util/atomic.h>

volatile ADCChannel adcChan;

#ifdef USE_ADC_INTERRUPT

ISR(ADC_vect, ISR_BLOCK)
{
    ADC_STORE_RESULT(ADMUX);
    ADMUX++;
    if(ADMUX & 0x08) {
        ADMUX = 0x00;
    }

    ADC_START_CONVERSION;
}

#else

inline void ADCStep()
{
    if(ADC_CONVERSION_DONE) {
        ADC_STORE_RESULT(ADMUX);
        ADMUX++;
        if(ADMUX & 0x08) {
            ADMUX = 0x00;
        }
        ADC_START_CONVERSION;
    }
}

#endif


/**
* Initialize ADC
*/
void ADCInit(void)
{
    ANALOGIN->control.PRESCALER = ADC_CLOCK_DIV_128;
    ANALOGIN->mux.VOLTAGE_REFERENCE_SELECT = ADC_REF_AREF;
    ANALOGIN->digitalInputDisable0.all = 0xFF; //Disable all digital input buffers for analog inputs
    ANALOGIN->digitalInputDisable1.all = 0xFF;
    ANALOGIN->control.ADC_ENABLE = TRUE;
    ANALOGIN->control.INTERRUPT_ENABLE = FALSE;    
    #ifdef USE_ADC_INTERRUPT
    ANALOGIN->control.INTERRUPT_ENABLE = TRUE;
    #endif
}


#ifdef DO_ANALOG_CONVERSION
void ADCStoreResult(ADCChannel chan)
{
    switch(chan)
    {
        case ADC_BATTERY:
        devInfo.batterymV = (uint16_t)((456994*(uint32_t)(ANALOGIN->data) - 13207900) >> 16);
        break;
        case ADC_ACCPWR:
        devInfo.regulatormV = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_USBD_P:
        devInfo.usbDataPmV = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_USBD_N:
        devInfo.usbDataNmV = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_DEVICEDET:
        devInfo.deviceDetectmV = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_M1CURRENT:
        // FIXME: this conversion is wrong
        devInfo.m1CurrentmA = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_M2CURRENT:
        // FIXME: this conversion is wrong
        devInfo.m2CurrentmA = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
        case ADC_M3CURRENT:
        // FIXME: this conversion is wrong
        devInfo.m3CurrentmA = (uint16_t)((216705*(uint32_t)(ANALOGIN->data) - 2960571) >> 16);
        break;
    }
}
#endif //DO_ANALOG_CONVERSION


/// Blocking function that reads all ADC values and stores them in devInfo
void ADCUpdateAll()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for(int i = 0; i < ADC_MAX_CHANNELS; i++) {
            ADC_SET_CHAN(i);
            ADC_START_CONVERSION;
            ADC_WAIT_FOR_CONVERSION;
            ADC_STORE_RESULT(i);
        }
    }
}

