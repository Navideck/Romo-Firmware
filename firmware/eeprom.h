/*
* eeprom.h
*
* Created: 8/15/2012 1:25:13 AM
*  Author: Aaron Solochek
*/

#ifndef EEPROM_H_
#define EEPROM_H_

#include <avr/eeprom.h>
#include "EEPROMDefs.h"


void EEPROMInit();
void ClearEEPROMProgramFlashFlag(void) __attribute__((naked)) __attribute__ ((section(".init3")));
void ReadSerialNumber(char *serial);
void WriteSerialNumber(char *serial);


#endif /* EEPROM_H_ */