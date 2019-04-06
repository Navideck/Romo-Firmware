/*
 * romo3.h
 *
 * Created: 10/4/2012 1:43:21 PM
 *  Author: Dan Kane
 */ 

#ifndef ROMO3_H_
#define ROMO3_H_

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BAUD                        57600

#define FW_VER_MAJOR				0
#define FW_VER_MINOR				0
#define FW_VER_REVISION				3

#define LEDS_ENABLE                 sbi(DDRB, DDRB0)
#define LEDS_ON						sbi(PORTB, PORTB0)
#define LEDS_OFF					cbi(PORTB, PORTB0)
#define LEDS_TOGGLE					sbi(PINB, PINB0)

#define CPRESET_ENABLE              sbi(PORTC, PORTC6)

#endif /* ROMO3_H_ */