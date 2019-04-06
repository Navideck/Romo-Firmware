/*
* commands.c
*
* Created: 10/7/2012 4:33:46 PM
*  Author: Dan Kane
*/

#include "main.h"
#include "commands.h"
#include "eeprom.h"
#include "pwm.h"
#include "mfi.h"
#include "charging.h"
#include "flags.h"
#include "leds.h"
#include "timers.h"
#include "ringbuffer.h"
#include "BoN_mode.h"
#include "dance.h"
#include "SerialProtocol.h"
#include "InfoTypes.h"
#include "ParameterTypes.h"
#include <util/delay.h>

uint8_t pCommandData[CMD_MAX_PAYLOAD];

readCommandState rcState;
uint8_t rcDataIndex;


void ReadCommandStep()
{
    uint8_t ch = 0;
    
    // if more than 1 second has elapsed since last character and we are in the middle of a command
    // reset the state
    if((rcState != RCS_INIT) && (intCnt0 > MS_TO_TICKS(1000))) {
        rcState = RCS_INIT;
        if(devInfo.mode == RMDeviceModeDebug) {
            MutexLockOutBuf();
            PutByteOutBuf(NAK);
            MutexUnlockOutBuf();
        }
    }
    while(BytesUsedInBuf()) {
        intCnt0 = 0; // always reset inter-character timeout

        switch(rcState) {
            case RCS_INIT:
            MutexLockInBuf();
            ch = GetByteInBuf();
            MutexUnlockInBuf();

            if(ch == STX) {
                rcState = RCS_GOT_STX;
                } else {
                if(devInfo.mode == RMDeviceModeDebug) {
                    MutexLockOutBuf();
                    PutByteOutBuf(NAK);
                    MutexUnlockOutBuf();
                }
                continue;
            }
            break;

            case RCS_GOT_STX: // got STX so now read CMD
            MutexLockInBuf();
            pCommandData[0] = GetByteInBuf();
            MutexUnlockInBuf();
            rcState = RCS_GOT_CMD;
            break;

            case RCS_GOT_CMD: // got CMD so now read LEN
            MutexLockInBuf();
            pCommandData[1] = GetByteInBuf();
            MutexUnlockInBuf();
            
            rcDataIndex = 0;
            rcState = RCS_GOT_LEN; // got LEN so now read DATA
            break;

            case RCS_GOT_LEN: // read LEN
            if((rcDataIndex < pCommandData[1]) &&  //there is still more data to read
            (rcDataIndex < IPOD_COMMAND_PAYLOAD_SIZE-2)) {
                MutexLockInBuf();
                pCommandData[(rcDataIndex++) +2] = GetByteInBuf();
                MutexUnlockInBuf();
                } else if(rcDataIndex < IPOD_COMMAND_PAYLOAD_SIZE-2) { //no more data to read
                rcState = RCS_GOT_DATA;
                } else { //ran out of space in the buffer
                rcState = RCS_INIT;
                if(devInfo.mode == RMDeviceModeDebug) {
                    MutexLockOutBuf();
                    PutByteOutBuf(NAK);
                    MutexUnlockOutBuf();
                }
                return;
            }
            break;

            case RCS_GOT_DATA: // read data
            MutexLockInBuf();
            ch = GetByteInBuf();
            MutexUnlockInBuf();

            if(ch == ETX) {
                rcState = RCS_GOT_ETX;
                return;
                } else {
                if(devInfo.mode == RMDeviceModeDebug) {
                    MutexLockOutBuf();
                    PutByteOutBuf(NAK);
                    MutexUnlockOutBuf();
                }
                rcState = RCS_INIT;
                return;
            }
            break;
            case RCS_GOT_ETX: // I really shouldn't get here
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Process the first byte of <data> as a command, and set the passed references
// to the size of the response to the command, and the response data.
// Return the number of bytes read out of <data>
//////////////////////////////////////////////////////////////////////////
uint8_t ProcessCommand(uint8_t *data, uint8_t *ackPayloadSize, uint8_t *cmdAck)
{
    int16_t lSpeed;
    int16_t rSpeed;
    int16_t tSpeed;
    
    if(!ackPayloadSize) {
        uint8_t _ackPayloadSize;
        ackPayloadSize = &_ackPayloadSize;
    }
    
    if(!cmdAck) {
        uint8_t _cmdAck[CMD_MAX_PAYLOAD+4];
        cmdAck = _cmdAck;
    }
    
    // We will always be at least 3 bytes
    // <ACK|NAK> <length> <CMD> <response data>
    if(devInfo.mode == RMDeviceModeRun) {
        *ackPayloadSize = 3;
        cmdAck[0] = RMCommandFromRobotNak;
        cmdAck[2] = data[0];
        } else {
        *ackPayloadSize = 1;
    }
        
    switch(data[0]) {
        case CMD_INITIALIZE:
        // Anything?
        break;
        
        case CMD_SET_LEDS_OFF:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            ledControl.mode = RMLedModeOff;
            LEDS_OFF;
        }
        break;
        
        case CMD_SET_LEDS_PWM:
        case CMD_SET_LEDS_PWM_OLD:
        if(data[1] == 1) {
            cmdAck[0] = RMCommandFromRobotAck;
            LEDSetPWM(data[2]);
        }
        break;

        case CMD_SET_LEDS_BLINK_LONG:
        if(data[1] == 8) {
            cmdAck[0] = RMCommandFromRobotAck;
            //            LEDSetBlink(Read32(data, 2), Read32(data, 6));
            LEDSetBlink(Read16(data, 4), Read16(data, 8));
        }
        break;

        case CMD_SET_LEDS_BLINK:
        if(data[1] == 4) {
            cmdAck[0] = RMCommandFromRobotAck;
            LEDSetBlink(Read16(data, 2), Read16(data, 4));
        }
        break;

        case CMD_SET_LEDS_PULSE:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            LEDSetPulse(data[2], data[3]);
        }
        break;
        
        case CMD_SET_LEDS_HALFPULSEUP:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            LEDSetHalfPulseUp(data[2], data[3]);
        }
        break;
        
        case CMD_SET_LEDS_HALFPULSEDOWN:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            LEDSetHalfPulseDown(data[2], data[3]);
        }
        break;

        case CMD_SET_MOTORS:
        if(data[1] == 6) {
            cmdAck[0] = RMCommandFromRobotAck;
            lSpeed = Read16(data, 2);
            rSpeed = Read16(data, 4);
            tSpeed = Read16(data, 6);
            
            SetLeftPWM(lSpeed);
            SetRightPWM(rSpeed);
            SetTiltPWM(tSpeed);
        }
        break;

        case CMD_SET_MOTOR_LEFT:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            lSpeed = Read16(data, 2);
            SetLeftPWM(lSpeed);
        }
        break;

        case CMD_SET_MOTOR_RIGHT:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            rSpeed = Read16(data, 2);
            SetRightPWM(rSpeed);
        }
        break;

        case CMD_SET_MOTOR_TILT:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            tSpeed = Read16(data, 2);
            SetTiltPWM(tSpeed);
        }
        break;
        case CMD_WRITE_EEPROM:
        if(data[1] < 130) {
            cmdAck[0] = RMCommandFromRobotAck;
            eeprom_write_block((void*)&(data[4]), (void *)Read16(data, 2), data[1]-2);
        }
        break;
        
        case CMD_READ_EEPROM:
        if(data[1] == 3) {
            cmdAck[0] = RMCommandFromRobotAck;
            eeprom_read_block((void*)(cmdAck+*ackPayloadSize), (const void *)Read16(data, 2), data[4]);
            *ackPayloadSize += data[4];
        }
        break;

        case CMD_GET_VITALS:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            // Battery Status
            cmdAck[(*ackPayloadSize)++] = (uint8_t)(devInfo.analogValues[ADC_BATTERY]>>8);
            cmdAck[(*ackPayloadSize)++] = (uint8_t)devInfo.analogValues[ADC_BATTERY];
            // Charging State
            cmdAck[(*ackPayloadSize)++] = (uint8_t)chargeInfo.chargerState;
        }
        break;
        
        case CMD_GET_MOTOR_CURRENT:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = (uint8_t)(devInfo.analogValues[ADC_M2CURRENT]>>8);   // Left motor
            cmdAck[(*ackPayloadSize)++] = (uint8_t)devInfo.analogValues[ADC_M2CURRENT];
            cmdAck[(*ackPayloadSize)++] = (uint8_t)(devInfo.analogValues[ADC_M1CURRENT]>>8);   // Right motor
            cmdAck[(*ackPayloadSize)++] = (uint8_t)devInfo.analogValues[ADC_M1CURRENT];
            cmdAck[(*ackPayloadSize)++] = (uint8_t)(devInfo.analogValues[ADC_M3CURRENT]>>8);   // Tilt motor
            cmdAck[(*ackPayloadSize)++] = (uint8_t)devInfo.analogValues[ADC_M3CURRENT];
        }
        break;

        case CMD_GET_BATTERY_STATUS:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            // Return analog value for battery?
            cmdAck[(*ackPayloadSize)++] = (uint8_t)(devInfo.analogValues[ADC_BATTERY]>>8);
            cmdAck[(*ackPayloadSize)++] = (uint8_t)devInfo.analogValues[ADC_BATTERY];
        }
        break;

        case CMD_GET_CHARGING_STATE:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = (uint8_t)chargeInfo.chargerState;
        }
        break;
        
        case CMD_SET_MODE:
        if(data[1] == 1) {
            cmdAck[0] = RMCommandFromRobotAck;
            devInfo.mode = data[2];
        }
        break;
        
        case CMD_SET_WATCHDOG:
        if(data[1] == 1) {
            cmdAck[0] = RMCommandFromRobotAck;
            wdt_enable(data[2]);
        }
        break;
        
        case CMD_DISABLE_WATCHDOG:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            wdt_disable();
        }
        break;
        
        case CMD_SOFT_RESET:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            STOP_ALL();
			gotReset = TRUE;
        }
        break;
        
        case CMD_ENTER_BON_MODE:
        STOP_ALL();
        BoNMain();
        break;
        
        case CMD_ENTER_DANCE_MODE:
        STOP_ALL();
        ledControl.mode = RMLedModePWM;
        DanceMain();
        break;
        
        case CMD_SET_DEV_CHARGE_ENABLE:
        if(data[1] == 1) {
            cmdAck[0] = RMCommandFromRobotAck;
            chargeInfo.iDeviceChargerState = data[2];
        }
        break;
        
        case CMD_SET_DEV_CHARGE_CURRENT:
        if(data[1] == 2) {
            cmdAck[0] = RMCommandFromRobotAck;
            chargeInfo.iDeviceChargingCurrent = Read16(data, 2);
        }
        break;
        
        case CMD_GET_FIRMWARE_VERSION:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = FIRMWARE_VERSION_MAJOR;
            cmdAck[(*ackPayloadSize)++] = FIRMWARE_VERSION_MINOR;
            cmdAck[(*ackPayloadSize)++] = FIRMWARE_VERSION_REVISION;
        }
        break;
        
        case CMD_GET_BOOTLOADER_VERSION:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = devInfo.bootloaderVersion.major;
            cmdAck[(*ackPayloadSize)++] = devInfo.bootloaderVersion.minor;
        }
        break;
        
        case CMD_GET_HARDWARE_VERSION:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = devInfo.hardwareVersion.major;
            cmdAck[(*ackPayloadSize)++] = devInfo.hardwareVersion.minor;
            cmdAck[(*ackPayloadSize)++] = devInfo.hardwareVersion.revision;
        }
        break;
        
        case STK_LEAVE_PROGMODE:
        if(data[1] == 0) {
            cmdAck[0] = RMCommandFromRobotAck;
        }
        break;
        
        // this is for the comm test and assumes no charger is present
        case RMCommandToRobotCheckResistor:
        if(data[1] == 0) {
            GPIO->portA.pullup.P5 = PULLUP_ENABLED;
            GPIO->portA.direction.P6 = OUTPUT;
            _delay_ms(10);
            ADC_SET_CHAN(ADC_USBD_P);
            ADC_START_CONVERSION;
            ADC_WAIT_FOR_CONVERSION;
            
            devInfo.analogValues[ADC_USBD_P] = ADCW;
            
            GPIO->portA.pullup.P5 = PULLUP_DISABLED;
            GPIO->portA.direction.P6 = INPUT;
            
            cmdAck[0] = RMCommandFromRobotAck;
            cmdAck[(*ackPayloadSize)++] = HIGH_BYTE(devInfo.analogValues[ADC_USBD_P]);
            cmdAck[(*ackPayloadSize)++] = LOW_BYTE(devInfo.analogValues[ADC_USBD_P]);
        }
        break;
        
        case RMCommandToRobotReadInfo:
        if(data[1] != 0) {
            cmdAck[0] = RMCommandFromRobotAck;
            
            switch((RMInfoType)data[2]) {
                case RMInfoTypeMotorCurrent:
                case RMInfoTypeMotorVelocity:
                case RMInfoTypeMotorPosition:
                case RMInfoTypeMotorPWM:
                case RMInfoTypeMotorTorque:
                case RMInfoTypeMotorTemp:
                break;
                
                case RMInfoTypeAllAnalog:
                for(int i=0; i < 8; i++) {
                    cmdAck[(*ackPayloadSize)++] = HIGH_BYTE(devInfo.analogValues[i]);
                    cmdAck[(*ackPayloadSize)++] = LOW_BYTE(devInfo.analogValues[i]);
                }
                break;
                
                case RMInfoTypeNull:
                default:
                break;
            }
        }
        break;
        
        case RMCommandToRobotReadParameter:
        break;
        
        case RMCommandToRobotSetParameter:
        break;
        
        
        default:
        break;
    }

    if(devInfo.mode == RMDeviceModeRun) {
        cmdAck[1] = *ackPayloadSize-2;
    } else if(devInfo.mode == RMDeviceModeDebug) {
        MutexLockOutBuf();
        for(int i=0; i < *ackPayloadSize; i++) {
            PutByteOutBuf(cmdAck[i]);
        }
        MutexUnlockOutBuf();
    }
    
    rcState = RCS_INIT;
    return data[1]+2;
}
