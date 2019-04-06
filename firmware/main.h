/*
* main.h
*
* Created: 8/13/2012 8:16:24 PM
*  Author: Aaron Solochek
*/


#ifndef MAIN_H_
#define MAIN_H_

#include "adc.h"
#include "charging.h"
#include "DeviceModes.h"
#include "DeviceType.h"
#include <inttypes.h>
#include <math.h>

#define FIRMWARE_VERSION_MAJOR			    1
#define FIRMWARE_VERSION_MINOR				1
#define FIRMWARE_VERSION_REVISION			9
#define devFlags                            GPIOR0
#define needsReset                          ((volatile devFlags1*)&GPIOR0)->reset_f
#define SOFT_RESET                          do{wdt_enable(WDTO_15MS); for(;;); } while(0)
#define HARD_RESET                          GPIO->portA.direction.P0 = OUTPUT;

#if defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324PA__)
#define BOOTLOADER_START_ADDRESS            0x7000
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284A__)
#define BOOTLOADER_START_ADDRESS            0x7000
#endif

#ifdef DO_ANALOG_CONVERSION

#define NORMAL_MODE_LOW_VOLTAGE             -0200
#define NORMAL_MODE_HI_VOLTAGE              0200
#define BON_MODE_LOW_VOLTAGE                0700
#define BON_MODE_HI_VOLTAGE                 1200
#define DANCE_MODE_DEVICE_DETECT_LOW        1700
#define DANCE_MODE_DEVICE_DETECT_HIGH       2200

#else

#define RUN_MODE_DEVICE_DETECT_LOW          0
#define RUN_MODE_DEVICE_DETECT_HIGH         74   // ~0.2V
#define DEBUG_MODE_DEVICE_DETECT_LOW        256  // ~0.8V
#define DEBUG_MODE_DEVICE_DETECT_HIGH       377  // ~1.2V
#define DANCE_MODE_DEVICE_DETECT_LOW        558  // ~1.8V
#define DANCE_MODE_DEVICE_DETECT_HIGH       679  // ~2.2V
#define CHARGE_MODE_DEVICE_DETECT_LOW       984  // ~3.2V 

#define RUN_MODE_ACC_POWER_LOW              930  // ~3.0V
#define RUN_MODE_ACC_POWER_HIGH             1023 // ~3.3V
#define DEBUG_MODE_ACC_POWER_LOW            713  // ~2.3V
#define DEBUG_MODE_ACC_POWER_HIGH           837  // ~2.7V
#define DANCE_MODE_ACC_POWER_LOW            558  // ~1.8V
#define DANCE_MODE_ACC_POWER_HIGH           679  // ~2.2V

#define LOW_VOLTAGE_THRESHOLD				74   // ~0.2V
#define HIGH_VOLTAGE_THRESHOLD				930  // ~3.0V
#define LIGHTNING_DETECT_LOW				745  // ~2.4V
#define LIGHTNING_DETECT_HIGH				930  // ~3V
#define LIGHTNING_TEST_DELTA				74   // ~0.2V

#endif //DO_ANALOG_CONVERSION



struct devInfo_t {
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t revision;
    } hardwareVersion;
    struct {
        uint8_t major;
        uint8_t minor;
    } bootloaderVersion;
    char serialNumber[20];
    RMDeviceMode previousMode;
    RMDeviceMode mode;
    RMDeviceType type;
    uint16_t analogValues[ADC_MAX_CHANNELS];
};


extern volatile struct devInfo_t devInfo;
extern volatile int16_t deviceDet;
extern volatile int16_t accPower;
extern uint16_t bootloaderSessionID;
extern uint16_t bootloaderTransactionID;
extern uint8_t gotReset;

void WDTInit(void) __attribute__((naked)) __attribute__ ((section(".init3")));



#endif /* MAIN_H_ */