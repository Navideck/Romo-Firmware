/*
 * commands.h
 *
 * Created: 10/1/2012 2:23:31 PM
 *  Author: Dan Kane
 */ 


#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "util.h"
#include "MFI/mfi_config.h"
#include "SerialProtocol.h"

//////////////////////////////////////////////////////////////////////////
// Romo and STK500 Commands
//////////////////////////////////////////////////////////////////////////

#define STX                         0x02
#define ETX                         0x03

#define CMD_ENTER_BON_MODE          0x05
#define ACK                         0x06
#define CMD_ENTER_DANCE_MODE        0x07
#define NAK                         0x15

#define CMD_INITIALIZE              0x21
#define CMD_SET_LEDS                0x22

#define CMD_SET_MOTORS              0x23
#define CMD_SET_MOTOR_LEFT          0x24
#define CMD_SET_MOTOR_RIGHT         0x25
#define CMD_SET_MOTOR_TILT          0x26

// 0x30 is reserved for STK500
#define CMD_SET_TRIM_PWM            0x30
#define CMD_SET_TRIM_CURRENT        0x31
#define CMD_GET_TRIM                0x32
#define CMD_SET_TRIM_FLAG           0x33
//#define CMD_READ_EEPROM             0x31
//#define CMD_WRITE_EEPROM            0x32

#define CMD_GET_VITALS              0x34
#define CMD_GET_MOTOR_CURRENT       0x35
#define CMD_GET_BATTERY_STATUS      0x36
#define CMD_GET_CHARGING_STATE      0x37
#define CMD_SET_WATCHDOG            0x38
#define CMD_SET_LEDS_BLINK_LONG     0x39
#define CMD_SET_DEV_CHARGE_ENABLE   0x3A
#define CMD_SET_DEV_CHARGE_CURRENT  0x3B
#define CMD_GET_FIRMWARE_VERSION    0x3C
#define CMD_GET_HARDWARE_VERSION    0x3D
#define CMD_GET_BOOTLOADER_VERSION  0x3E
#define CMD_ACK                     0x40
// 0x41 and 0x42 are reserved for STK500

#define CMD_SET_LEDS_PWM            0x43
#define CMD_SET_LEDS_OFF            0x44 
// 0x45 is reserved for STK500
#define CMD_SET_LEDS_PWM_OLD        0x45 //needs to change
#define CMD_SET_LEDS_BLINK          0x46
#define CMD_SET_LEDS_PULSE          0x47
#define CMD_SET_LEDS_HALFPULSEUP    0x48
#define CMD_SET_LEDS_HALFPULSEDOWN  0x49


// 0x50 and 0x51 are reserved for STK500
#define STK_LEAVE_PROGMODE          0x51
#define CMD_DISABLE_WATCHDOG        0x52 
#define CMD_SOFT_RESET              0x53 
#define CMD_SET_MODE                0x54


// 0x55 and 0x56 are reserved for STK500

#define CMD_READ_EEPROM             0x57
#define CMD_WRITE_EEPROM            0x58

// 0x64 is reserved for STK500

// 0x74 nad 0x75 are reserved for STK500


typedef enum readCommandState_t {
   RCS_INIT,
   RCS_GOT_STX,
   RCS_GOT_CMD,
   RCS_GOT_LEN,
   RCS_GOT_DATA,
   RCS_GOT_ETX
} readCommandState;



extern uint8_t pCommandData[IPOD_COMMAND_PAYLOAD_SIZE];
extern uint8_t ackHeaderSize;
extern readCommandState rcState;
extern uint8_t rcDataIndex;



void ReadCommandStep();
uint8_t ReadCommand();
uint8_t ProcessCommand(uint8_t* data, uint8_t *ackPayloadSize, uint8_t *cmdAck);

#endif /* COMMANDS_H_ */