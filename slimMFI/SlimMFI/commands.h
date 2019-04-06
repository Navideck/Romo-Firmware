/*
 * commands.h
 *
 * Created: 10/1/2012 2:23:31 PM
 *  Author: Dan Kane
 */ 


#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "util.h"
#include "mfi_config.h"

#define ACK_HEADER_SIZE             4       // Includes session ID (2 bytes), ack command, and payload length
#define CMD_MAX_PAYLOAD             12      // Maximum bytes for Romo Command parameters

//////////////////////////////////////////////////////////////////////////
// Romo Commands
//////////////////////////////////////////////////////////////////////////

#define STX                         0x02
#define ETX                         0x03

#define CMD_ENTER_BON_MODE          0x05

#define CMD_INITIALIZE              0x21
#define CMD_SET_LEDS                0x22
#define CMD_SET_MOTORS              0x23
#define CMD_SET_MOTOR_LEFT          0x24
#define CMD_SET_MOTOR_RIGHT         0x25
#define CMD_SET_MOTOR_TILT          0x26

#define CMD_SET_TRIM_PWM            0x30
#define CMD_SET_TRIM_CURRENT        0x31
#define CMD_GET_TRIM                0x32
#define CMD_SET_TRIM_FLAG           0x33

#define CMD_GET_MOTOR_CURRENT       0x35
#define CMD_GET_BATTERY_STATUS      0x36
#define CMD_GET_CHARGING_STATE      0x37

#define CMD_ACK                     0x40

#define CMD_SET_LEDS_OFF            0x44
#define CMD_SET_LEDS_PWM            0x45
#define CMD_SET_LEDS_BLINK          0x46
#define CMD_SET_LEDS_PULSE          0x47
/*
#define SET_MOTOR_RIGHT(x)          SetPWM(PWM0, -x)    // Right motor is backward
#define SET_MOTOR_LEFT(x)           SetPWM(PWM1, x)
#define SET_MOTOR_TILT(x)           SetPWM(PWM2, -x)    // positive x goes backward, so invert
*/
extern uint8_t pCommandData[IPOD_COMMAND_PAYLOAD_SIZE];

/*
#ifdef USE_READ_COMMAND_FSM

typedef enum readCommandState_t {
   RCS_INIT,
   RCS_GOT_STX,
   RCS_GOT_CMD,
   RCS_GOT_LEN,
   RCS_GOT_DATA,
   RCS_GOT_ETX
} readCommandState;


extern readCommandState rcState;
extern uint8_t rcDataIndex;

#endif
*/


// struct RomoCommand_t {
//     uint8_t command;
//     union {
//         uint8_t all[CMD_MAX_PAYLOAD];
//         // Initialize
//         struct {
//             uint8_t mode;
//             union {
//                 uint8_t pwm;
//                 uint16_t blinkRate;
//             };
//         } setLeds;
//         struct {
//             uint16_t left;
//             uint16_t right;
//             uint16_t tilt;
//         } setMotors;
//         struct {
//             uint16_t left;
//         } setMotorLeft;
//         struct {
//             uint16_t right;
//         } setMotorRight;
//         struct {
//             uint16_t tilt;
//         } setMotorTilt;
//         struct {
//             uint8_t trimLeft;
//             uint8_t trimRight;
//         } setTrim;
//     } parameters;
// };

//void ReadCommandStep();
//uint8_t ReadCommand();
uint8_t ProcessCommand(uint8_t* data);

#endif /* COMMANDS_H_ */