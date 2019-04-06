/*
* eeprom.c
*
* Created: 8/15/2012 1:25:04 AM
*  Author: Aaron Solochek
*/


#include "main.h"
#include "eeprom.h"
#include "registers.h"
#include <avr/io.h>



/**
* Initialize EEPROM
*/
void EEPROMInit(void)
{
    EEPROM->control.PROG_MODE = EEPROM_ERASE_AND_WRITE;
}

void ClearEEPROMProgramFlashFlag()
{
    eeprom_write_byte(EEPROM_PROGRAM_FLASH_ON_BOOT_ADDRESS, 0x00);
    return;
}

/// Read the serial number into the provided buffer, which must be at least 20 bytes
void ReadSerialNumber(char *serial)
{
    char buf1[20];
    char buf2[20];
    char buf3[20];
    uint8_t i = 0;
    uint8_t badChar = 0;

    // Read EEPROM data into buffers
    eeprom_read_block((void *)&buf1, EEPROM_SERIALNUMBER_ADDRESS, 20);
    eeprom_read_block((void *)&buf2, EEPROM_SERIALNUMBER_ADDRESS_ALT1, 20);
    eeprom_read_block((void *)&buf3, EEPROM_SERIALNUMBER_ADDRESS_ALT2, 20);

    buf1[19] = 0;

    // Check to see if the serial number exists in the alternate locations,
    // and if it does not, put it there
    for(i=0; i<19; i++) {
        if(buf2[i] < 0x20 || buf2[i] > 0x7D) {
            badChar++;
        }
    }
    
    if(badChar > 4) {
        eeprom_write_block(buf1, EEPROM_SERIALNUMBER_ADDRESS_ALT1, 20);
        badChar = 0;
    }
    
    for(i=0; i<19; i++) {
        if(buf3[i] < 0x20 || buf3[i] > 0x7D) {
            badChar++;
        }
    }
    
    if(badChar > 4) {
        eeprom_write_block(buf1, EEPROM_SERIALNUMBER_ADDRESS_ALT2, 20);
    }
    

    // For every character, choose a value that exists in both locations
    for(i=0; i<20; i++) {
        if(buf1[i] == buf2[i]) {
            serial[i] = buf2[i];
        }
        else if(buf1[i] == buf3[i]) {
            serial[i] = buf3[i];
        }
        else if(buf2[i] == buf3[i]) {
            serial[i] = buf3[i];
        }
        else {
            serial[i] = 0x23; //#
        }
    }

    return;
}


/// Write the serial number to EEPROM in 3 locations for increased reliability
void WriteSerialNumber(char *serial)
{
    eeprom_write_block((void *)serial, EEPROM_SERIALNUMBER_ADDRESS, 20);
    eeprom_write_block((void *)serial, EEPROM_SERIALNUMBER_ADDRESS_ALT1, 20);
    eeprom_write_block((void *)serial, EEPROM_SERIALNUMBER_ADDRESS_ALT2, 20);
}