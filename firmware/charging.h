/*
* charging.h
*
* Created: 10/9/2012 4:40:25 PM
*  Author: Aaron Solochek
*/


#ifndef CHARGING_H_
#define CHARGING_H_

#include "main.h"
#include "flags.h"
#include "timers.h"
#include "ChargingStates.h"

#define USB_CHARGER_MARK_VOLTAGE_LOW		680  // ~2.2V
#define USB_CHARGER_MARK_VOLTAGE_HI			891  // ~2.9V
#define USB_CHARGER_SPACE_VOLTAGE_LOW		434  // ~1.4V
#define USB_CHARGER_SPACE_VOLTAGE_HI		679  // ~2.2V
#define USB_CHARGER_VOLTAGE_HI_THRESHOLD	680  // ~2.2V
#define USB_CHARGER_VOLTAGE_LOW_THRESHOLD	434  // ~1.4V

#define CHARGE_LED_MONITOR_COUNTS           ((F_CPU/131072L)+(F_CPU/2000000L)) // 500ms + a bit

#define SET_CHARGE_CURRENT_208mA			cbi(PORTB, PORTB1); cbi(PORTB, PORTB2)
#define SET_CHARGE_CURRENT_436mA			sbi(PORTB, PORTB1); cbi(PORTB, PORTB2)
#define SET_CHARGE_CURRENT_708mA			cbi(PORTB, PORTB1); sbi(PORTB, PORTB2)
#define SET_CHARGE_CURRENT_936mA			sbi(PORTB, PORTB1); sbi(PORTB, PORTB2)

#define CHARGING_LED                        (!(PINC & _BV(PINC7)))


extern volatile uint8_t chargeLEDIntCnt;
extern volatile uint8_t chargeLEDChangeCnt;

#define chargeLED ((devFlags1*)&GPIOR0)->chargeLEDState_f
#define chargeLEDStable ((devFlags1*)&GPIOR0)->chargeLEDStable_f


typedef enum usbSourceCurrents_t {
    USB_SOURCE_500mA,
    USB_SOURCE_1000mA,
    USB_SOURCE_2100mA,
    USB_SOURCE_2400mA,
    USB_SOURCE_NONE,
    USB_SOURCE_UNKNOWN0,
    USB_SOURCE_UNKNOWN1
} usbSourceCurrent;


typedef enum chargingCurrents_t {
    CHARGING_CURRENT_208mA,
    CHARGING_CURRENT_436mA,
    CHARGING_CURRENT_708mA,
    CHARGING_CURRENT_936mA
} chargingCurrent;


struct chargeInfo_t {
    usbSourceCurrent chargerCurrentAvailable;
    chargingCurrent chargeCurrent;
    RMChargingState chargerState;
    RMChargingState previousChargerState;
    uint8_t chargeStateChangeCnt;
    uint8_t forceCharge;
    RMChargingState iDeviceChargerState;
    uint16_t iDeviceChargingCurrent;
};


extern volatile struct chargeInfo_t chargeInfo;

inline void ChargeStateDetectStep(void)
{
    // this should be true every 8.192ms @8MHz, 5.461ms @12MHz
    if(!intCnt1) {
        // monitor charging LED input
        // when the state changes, we monitor it for at least half a second
        if(!chargeLEDStable) {
            chargeLEDIntCnt++;
            if(CHARGING_LED != chargeLED) {
                chargeLEDChangeCnt++;
                chargeLED = CHARGING_LED;
            }
            
            if(chargeLEDIntCnt == CHARGE_LED_MONITOR_COUNTS) {
                if(chargeLEDChangeCnt) {
                    chargeInfo.chargeStateChangeCnt++;
                    if(chargeInfo.chargeStateChangeCnt >= 2) {
                       chargeInfo.chargerState = RMChargingStateError;
                    }
                } else {
                    chargeInfo.chargerState = chargeLED;
                    chargeInfo.chargeStateChangeCnt = 0;
                }
                
                chargeLEDStable = 1;
            }
        } else {
            chargeLEDStable = 0;
            chargeLEDIntCnt = 0;
            chargeLEDChangeCnt = 0;
        }
    }
}

void ChargingInit();
void ChargingStep();

#endif /* CHARGING_H_ */