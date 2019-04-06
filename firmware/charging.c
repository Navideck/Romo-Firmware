/*
* charging.c
*
* Created: 10/9/2012 4:48:30 PM
*  Author: Aaron Solochek
*/


#include "main.h"
#include "adc.h"
#include "leds.h"
#include "util.h"


volatile struct chargeInfo_t chargeInfo;
volatile uint8_t chargeLEDIntCnt;
volatile uint8_t chargeLEDChangeCnt;


void ChargingInit()
{
    chargeInfo.chargerState = RMChargingStateUnknown;
    chargeInfo.previousChargerState = RMChargingStateUnknown;
    chargeInfo.iDeviceChargerState = RMChargingStateOn;
    chargeInfo.iDeviceChargingCurrent = 0x0000;
    
    // Check the initial value of the charging LED to determine if we should be in charge mode
    if(CHARGING_LED && (devInfo.mode != RMDeviceModeDebug)) {
        chargeInfo.chargerState = RMChargingStateOn;
        devInfo.mode = RMDeviceModeCharge;
    }
}

void ChargingStep()
{
    chargeInfo.chargerCurrentAvailable = 0x0; // start with variable cleared
    // check for charger
    if(devInfo.analogValues[ADC_USBD_P] > USB_CHARGER_VOLTAGE_HI_THRESHOLD) {
        chargeInfo.chargerCurrentAvailable = 0x2; // set bit 1 high (high current options)
    } else if(devInfo.analogValues[ADC_USBD_P] < USB_CHARGER_VOLTAGE_LOW_THRESHOLD) {
        chargeInfo.chargerCurrentAvailable = 0x4; // set bit 2 high (no value detected)
    }
    // check for charger
    if(devInfo.analogValues[ADC_USBD_N] > USB_CHARGER_VOLTAGE_HI_THRESHOLD) {
        chargeInfo.chargerCurrentAvailable |= 0x1; // set bit 0 high
    } else if(devInfo.analogValues[ADC_USBD_N] < USB_CHARGER_VOLTAGE_LOW_THRESHOLD) {
        chargeInfo.chargerCurrentAvailable |= 0x4; // set bit 3 high (no value detected)
    }
    switch(chargeInfo.chargerCurrentAvailable) {
        case USB_SOURCE_2400mA:
        case USB_SOURCE_2100mA:
        chargeInfo.chargeCurrent = CHARGING_CURRENT_936mA;
        SET_CHARGE_CURRENT_708mA;
        break;

        case USB_SOURCE_1000mA:
        chargeInfo.chargeCurrent = CHARGING_CURRENT_436mA;
        SET_CHARGE_CURRENT_436mA;
        break;

        case USB_SOURCE_500mA:
        case USB_SOURCE_NONE:
        case USB_SOURCE_UNKNOWN0:
        case USB_SOURCE_UNKNOWN1:
        chargeInfo.chargeCurrent = CHARGING_CURRENT_208mA;
        SET_CHARGE_CURRENT_208mA;
        break;
    }    
}