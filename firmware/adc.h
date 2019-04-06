/*
* adc.h
*
* Created: 8/13/2012 8:22:53 PM
*  Author: Aaron Solochek
*/

#ifndef ADC_H_
#define ADC_H_


// Mappings of romo signals to ADC pins
typedef enum ADCChannels {
    ADC_ACCPWR,
    ADC_BATTERY,
    ADC_M1CURRENT,
    ADC_M2CURRENT,
    ADC_M3CURRENT,
    ADC_USBD_P,
    ADC_USBD_N,
    ADC_DEVICEDET
} ADCChannel;

#define ADC_MAX_CHANNELS			8
#define ADC_CONVERSION_DONE			bit_is_set(ADCSRA, ADIF)
#define ADC_WAIT_FOR_CONVERSION		loop_until_bit_is_set(ADCSRA, ADIF)
#define ADC_SET_CHAN(ch)			ADMUX = ch; sbi(ADMUX, REFS0)
#define ADC_START_CONVERSION		sbi(ADCSRA, ADSC)
#define ADC_STORE_RESULT(ch)		devInfo.analogValues[ch] = ANALOGIN->data


void ADCInit();
void ADCStep();
void ADCUpdateAll();
extern volatile ADCChannel adcChan;

#endif /* ADC_H_ */