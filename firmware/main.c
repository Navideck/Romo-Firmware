/*
* romo3.c
*
* Created: 8/13/2012 6:55:27 PM
*  Author: Aaron Solochek
*/

#include "main.h"
#include "util.h"
#include "registers.h"
#include "MFI/mfi_config.h"
#include "MFI/mfi.h"
#include "ringbuffer.h"
#include "adc.h"
#include "commands.h"
#include "current_control.h"
#include "dance.h"
#include "eeprom.h"
#include "pwm.h"
#include "uart.h"
#include "twi.h"
#include "timers.h"
#include "BoN_mode.h"
#include "leds.h"
#include "charging.h"
#include "MFI/mfi_ident.h"
#include "MFI/mfi_auth.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>


volatile struct devInfo_t devInfo;
volatile int16_t deviceDet;
volatile int16_t accPower;
volatile uint8_t intBuf[64];
volatile uint8_t intBufUsed;
volatile uint8_t intBufIndex;
uint8_t gotReset;
uint16_t bootloaderSessionID;
uint16_t bootloaderTransactionID;

// Timer interrupt at around 30kHz @8MHz, 46.875kHz @12MHz, 60kHz @16MHz
ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
    TimerStep();
    #ifdef DEBUG_TIMING
    sbi(PORTB, PORTB1);
    #endif

    // If there is space in the input buffer, then check if there are bytes that
    // need to be added to it
    if(BytesFreeInBuf()) {

        // If we have bytes saved in the interrupt buffer, these should be transferred
        // to the input buffer first
        if(intBufUsed && (intBufUsed < BytesFreeInBuf())) {
            if(MutexTryLockInBuf()) {
                for(intBufIndex=0; intBufIndex < intBufUsed; intBufIndex++) {
                    PutByteInBuf(intBuf[intBufIndex]);
                }
                MutexUnlockInBuf();
                intBufUsed = 0;
            }
        }
        
        // If we have received a new byte on the serial port, attempt to put it in the
        // input buffer, but if that fails, put it in the interrupt buffer and we'll
        // come back to that later
        if(UART1_RX_COMPLETE) {
            if(MutexTryLockInBuf()) {
                PutByteInBuf(UDR1);
                MutexUnlockInBuf();
            }
            else {
                if(intBufUsed < 64) {
                    intBuf[intBufUsed++] = UDR1;
                }
            }
            // Reset the ticks since last byte received counter
            packetTimer = 0;
        }
    }

    // Check for bytes in outBuf and send
    if(MutexTryLockOutBuf()) {
        if(UART1_EMPTY && BytesUsedOutBuf()) {
            UART1_TRANSMIT(GetByteOutBuf());
        }
        MutexUnlockOutBuf();
    }

    if(needsReset ||
    abs((int16_t)(deviceDet - devInfo.analogValues[ADC_DEVICEDET]) > 50) ||
    abs((int16_t)(accPower - devInfo.analogValues[ADC_ACCPWR]) > 50) ) {
        
        MutexLockOutBuf();
        while(BytesUsedOutBuf()) {
            if(UART1_EMPTY) {
                UART1_TRANSMIT(GetByteOutBuf());
            }
        }
        // No need to unlock the mutex since we're just resetting the MCU now

        cbi(PORTC, PORTC6); // start reset of coprocessor

        delay_us(15);
        sbi(PORTC, PORTC6); // restore coprocessor reset signal

        RegistersReset();
        
        // older bootloaders require this reset method
        if((devInfo.bootloaderVersion.major == 4) && (devInfo.bootloaderVersion.minor <= 9)) {
            __asm__ __volatile__ (
            "ldi r16, %[_REL]\n"
            "out %[_SPL], r16\n"
            "ldi r16, %[_REH]\n"
            "out %[_SPH], r16\n"
            "jmp %[_BL]\n" ::
            [_REL] "X" (LOW_BYTE(RAMEND)),
            [_SPL] "X" (_SFR_IO_ADDR(SPL)),
            [_REH] "X" (HIGH_BYTE(RAMEND)),
            [_SPH] "X" (_SFR_IO_ADDR(SPH)),
            [_BL] "X" (BOOTLOADER_START_ADDRESS)
            );
        }
        else {
            SOFT_RESET;
        }
    }
    
    ChargeStateDetectStep();
    LEDStep();
    #ifdef DEBUG_TIMING
    cbi(PORTB, PORTB1);
    #endif
}



// General setup
void RomoInit()
{
    cli();

    // Setup pins we care about
    GPIO->portC.pullup.P7 = PULLUP_ENABLED; // CHARGING_LED

    // Read EEPROM data
    ReadSerialNumber((char*)&devInfo.serialNumber);
    
    for(uint8_t i=0; i<19; i++) {
        if((devInfo.serialNumber[i] < 0x20) || (devInfo.serialNumber[i] > 0x7E)) {
            devInfo.serialNumber[i] = 0x23;
        }
    }
    devInfo.serialNumber[19] = 0x00;

    deviceDet = devInfo.analogValues[ADC_DEVICEDET];
    accPower = devInfo.analogValues[ADC_ACCPWR];

    // If device detect is 3.3, 30-pin charging w/o device
    if(deviceDet >= CHARGE_MODE_DEVICE_DETECT_LOW) {
        devInfo.mode = RMDeviceModeCharge;
        devInfo.type = RMDeviceTypeApple30Pin;
    }

    // If device detect is ~2, 30-pin in dance
    else if((deviceDet >= DANCE_MODE_DEVICE_DETECT_LOW) &&
    (deviceDet <= DANCE_MODE_DEVICE_DETECT_HIGH)) {
        devInfo.mode = RMDeviceModeDance;
        devInfo.type = RMDeviceTypeApple30Pin;
    }

    // If device detect is ~1, 30-pin in debug
    else if((deviceDet >= DEBUG_MODE_DEVICE_DETECT_LOW) &&
    (deviceDet <= DEBUG_MODE_DEVICE_DETECT_HIGH)) {
        devInfo.mode = RMDeviceModeDebug;
        devInfo.type = RMDeviceTypeApple30Pin;
    }

    // If device detect 0, we might have a lightning robot
    else if((deviceDet >= RUN_MODE_DEVICE_DETECT_LOW) &&
    (deviceDet <= RUN_MODE_DEVICE_DETECT_HIGH)) {
        
        // If ACCPWR is ~2V, we are in dance mode
        if((accPower >= DANCE_MODE_ACC_POWER_LOW) &&
        (accPower <= DANCE_MODE_ACC_POWER_HIGH)) {
            devInfo.mode = RMDeviceModeDance;
            devInfo.type = RMDeviceTypeAppleLightning;
        }
        
        // If ACCPWR is ~2.5V, we are in debug mode
        else if((accPower >= DEBUG_MODE_ACC_POWER_LOW) &&
        (accPower <= DEBUG_MODE_ACC_POWER_HIGH)) {
            devInfo.mode = RMDeviceModeDebug;
            devInfo.type = RMDeviceTypeAppleLightning;
        }
        
        else {
            // Check for apple charger
            GPIO->portA.pullup.P5 = PULLUP_ENABLED;
            _delay_ms(10);
            ADC_SET_CHAN(ADC_USBD_P);
            ADC_START_CONVERSION;
            ADC_WAIT_FOR_CONVERSION;
            
            // There must be an apple charger pulling the voltage down
            if(ADCW < HIGH_VOLTAGE_THRESHOLD) {
                int16_t previousADCValue = ADCW;
                
                GPIO->portA.pullup.P5 = PULLUP_DISABLED;
                _delay_ms(10);
                ADC_START_CONVERSION;
                ADC_WAIT_FOR_CONVERSION;
                
                // If USBD+ voltage drops to ~0 once the pullup is disabled
                // then we are not hooked to an apple charger
                if(ADCW < LOW_VOLTAGE_THRESHOLD) {
                    devInfo.type = RMDeviceTypeApple30Pin;
                    
                    // See if we are hooked to some non-apple charger
                    if(bit_is_clear(PORTC, PINC7)) {
                        devInfo.mode = RMDeviceModeCharge;
                    }
                    else {
                        devInfo.mode = RMDeviceModeRun;
                    }
                    goto finish;
                }
                
                GPIO->portA.pullup.P5 = PULLUP_ENABLED;
                GPIO->portA.direction.P6 = OUTPUT;
                _delay_ms(10);
                ADC_SET_CHAN(ADC_USBD_P);
                ADC_START_CONVERSION;
                ADC_WAIT_FOR_CONVERSION;
                
                // Special check for device type given that we are hooked to an apple charger
                if((int16_t)(previousADCValue - ADCW) > LIGHTNING_TEST_DELTA) {
                    devInfo.type = RMDeviceTypeAppleLightning;
                    devInfo.mode = RMDeviceModeCharge;
                }
                // USBD+ did not change after we drove USBD- to 0, so we are not lightning
                else {
                    devInfo.type = RMDeviceTypeApple30Pin;
                    devInfo.mode = RMDeviceModeCharge;
                }
            }

            // If we are charging, it is a non-apple charger, or we are not charging
            else {
                // Check to see if we are connected to a charger
                if(bit_is_clear(PORTC, PINC7)) {
                    devInfo.mode = RMDeviceModeCharge;
                }
                else {
                    devInfo.mode = RMDeviceModeRun;
                }

                // Drive USBD+ low, read USBD-
                GPIO->portA.pullup.P5 = PULLUP_DISABLED;
                GPIO->portA.direction.P5 = OUTPUT;
                _delay_ms(10);
                ADC_SET_CHAN(ADC_USBD_N);
                ADC_START_CONVERSION;
                ADC_WAIT_FOR_CONVERSION;

                // If USBD- is low, we are half way to determining lightning
                if(ADCW <= LIGHTNING_DETECT_LOW) {
                    devInfo.type = RMDeviceTypeAppleLightning;
                }

                // Enable the USBD- pullup. If USBD- goes all the way up, it is isolated,
                // if the lightning ID resistor is present, USBD- will go up into the 2.4-3.0 range
                GPIO->portA.pullup.P6 = PULLUP_ENABLED;
                _delay_ms(10);
                ADC_START_CONVERSION;
                ADC_WAIT_FOR_CONVERSION;

                // Check if USBD- is in the 2.4-3.0 range. We either confirm we are lightning by
                // being in this range, or if we are still unknown and in this range, we are 30-pin
                if((ADCW < LIGHTNING_DETECT_HIGH) &&
                (ADCW > LIGHTNING_DETECT_LOW)) {
                    if(devInfo.type != RMDeviceTypeAppleLightning) {
                        devInfo.type = RMDeviceTypeApple30Pin;
                    }
                }
                // If we fall outside of the lightning range, we are 30-pin
                else {
                    devInfo.type = RMDeviceTypeApple30Pin;
                }
                
            }
        }
    }
    //We don't know what's happening
    else {
        devInfo.mode = RMDeviceModeUnknown;
    }

    finish:
    GPIO->portA.output.P5 = 0;
    GPIO->portA.output.P6 = 0;
    GPIO->portA.direction.P5 = INPUT;
    GPIO->portA.direction.P6 = INPUT;
    
    
    devInfo.previousMode = devInfo.mode;

    devInfo.hardwareVersion.major = eeprom_read_byte(EEPROM_HW_MAJOR_VERSION_ADDRESS);
    devInfo.hardwareVersion.minor = eeprom_read_byte(EEPROM_HW_MINOR_VERSION_ADDRESS);
    devInfo.hardwareVersion.revision = eeprom_read_byte(EEPROM_HW_REVISION_VERSION_ADDRESS);
    devInfo.bootloaderVersion.major = eeprom_read_byte(EEPROM_BOOTLOADER_MAJOR_VERSION_ADDRESS);
    devInfo.bootloaderVersion.minor = eeprom_read_byte(EEPROM_BOOTLOADER_MINOR_VERSION_ADDRESS);
    
    MCUCR = (1<<IVCE);
    MCUCR = 0;
    sei();
}


void WDTInit()
{
    MCUSR = 0;
    wdt_disable();
}


int main(void)
{
    // first grab the transactionID and sessionID that the bootloader last used
    bootloaderSessionID = GPIOR0;
    bootloaderTransactionID = (GPIOR1 << 8) | GPIOR2;

    GPIOR0 = 0;
    GPIOR1 = 0;
    GPIOR2 = 0;
    
    GPIO->portB.direction.P0 = OUTPUT;      // LEDCHAN0
    LEDS_ON;

    RegistersInit();
    UART0Init();
    UART1Init();
    ADCInit();
    ADCUpdateAll();
    RomoInit();
    ChargingInit();
    TimersInit();
    
    LEDS_OFF;
    UART0TransmitCString("Starting\n");

    FlushInBuf();
    
    //devInfo.mode = RM_DEV_MODE_RUN;
    //devInfo.mode = RM_DEV_MODE_DEBUG;
    //devInfo.mode = RM_DEV_MODE_DANCE;
    //devInfo.mode = RM_DEV_MODE_CHARGE;
    //devInfo.mode = RM_DEV_MODE_UNKNOWN;
    switch(devInfo.mode) {
        case RMDeviceModeDebug:
        GPIO->portC.direction.P6 = OUTPUT;     // CPRESET

        //some way to differentiate BoN from DEBUG
        if(bit_is_set(PINB, PINB1)) {
            BoNMain();
        }
        else {
            GPIO->portB.direction.P0 = OUTPUT;      // LEDCHAN0
            GPIO->portB.direction.P1 = OUTPUT;      // CHARGECURRENTSEL0
            GPIO->portB.direction.P2 = OUTPUT;      // CHARGECURRENTSEL1
            PWMInit();
            LEDSetBlink(50, 200);
        }
        break;

        case RMDeviceModeDance:
        GPIO->portB.direction.P0 = OUTPUT;      // LEDCHAN0
        PWMInit();
        DanceMain();
        break;

        case RMDeviceModeRun:
        PWMInit();
        
        case RMDeviceModeCharge:
        GPIO->portC.direction.P6 = OUTPUT;     // CPRESET
        GPIO->portB.direction.P0 = OUTPUT;     // LEDCHAN0
        GPIO->portB.direction.P1 = OUTPUT;     // CHARGECURRENTSEL0
        GPIO->portB.direction.P2 = OUTPUT;     // CHARGECURRENTSEL1
        MFIInit();
        break;

        case RMDeviceModeUnknown:
        GPIO->portB.direction.P0 = OUTPUT;     // CPRESET
        LEDSOS();

        break;
        default:
        break;
    }

    while(1) {
        // main loop
        if(loopTriggered) {
            #ifdef DEBUG_TIMING
            sbi(PORTB, PORTB2);
            #endif
            
            #ifndef USE_ADC_INTERRUPT
            ADCStep(); //read ADC
            #endif

            ChargingStep();

			// Trigger a reset if we received (and by now composed an ACK packet) a reset command
			if(gotReset) {
				needsReset = TRUE;
			}

            switch(devInfo.mode) {
                case RMDeviceModeRun:
                MFITasks();
                MFIIdentTasks();
                MFIAuthTasks();

                if(chargeInfo.chargerState == RMChargingStateOn) {
                    devInfo.mode = RMDeviceModeCharge;
                }

                break;

                case RMDeviceModeDebug:
                ReadCommandStep();
                if(rcState == RCS_GOT_ETX)
                ProcessCommand((uint8_t*)&pCommandData, NULL, NULL);
                break;

                case RMDeviceModeCharge:
                MFITasks();
                MFIIdentTasks();
                MFIAuthTasks();
                STOP_ALL();
                // More specific state transition checking here
                //if(chargeInfo.chargerState != chargeInfo.previousChargerState) {
                switch(chargeInfo.chargerState) {
                    case RMChargingStateOn:
                    switch(chargeInfo.chargeCurrent) {
                        case CHARGING_CURRENT_208mA:
                        LEDSetPulse(1,1);
                        break;

                        case CHARGING_CURRENT_436mA:
                        LEDSetPulse(1,2);
                        break;

                        case CHARGING_CURRENT_708mA:
                        LEDSetPulse(1,3);
                        break;

                        case CHARGING_CURRENT_936mA:
                        LEDSetPulse(1,4);
                        break;
                    }
                    break;

                    case RMChargingStateOff:
                    LEDSetPWM(55);
                    break;

                    case RMChargingStateError:
                    LEDSetBlink(500, 500);
                    break;

                    case RMChargingStateUnknown:
                    default:
                    break;
                }
                //chargeInfo.previousChargerState = chargeInfo.chargerState;
                //}
                break;

                case RMDeviceModeDance:
                case RMDeviceModeUnknown:
                default:
                STOP_ALL();
                break;
            }
            
            loopTrigger = 0;
            loopTriggered = 0;
            #ifdef DEBUG_TIMING
            cbi(PORTB, PORTB2);
            #endif
        }
    }
}