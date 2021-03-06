﻿///////////////////////////////////////////////////////////////////
//
// Romotive Romo MFI/stk500 bootloader
//
// Code builds on code, libraries and optimizations from:
//   stk500boot.c          by Jason P. Kyle
//   Arduino bootloader    http://arduino.cc
//   Spiff's 1K bootloader http://spiffie.org/know/arduino_1k_bootloader/bootloader.shtml
//   avr-libc project      http://nongnu.org/avr-libc
//   Adaboot               http://www.ladyada.net/library/arduino/bootloader.html
//   AVR305                Atmel Application Note
//   Optiboot              http://optiboot.googlecode.com
//
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program; if not, write
// to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// License can be viewed at
// http://www.fsf.org/licenses/gpl.txt
//
///////////////////////////////////////////////////////////////////


#define OPTIBOOT_MAJVER 4
#define OPTIBOOT_MINVER 10

#define MAKESTR(a) #a
#define MAKEVER(a, b) MAKESTR(a*256+b)

asm("  .section .version\n"
"optiboot_version:  .word " MAKEVER(OPTIBOOT_MAJVER, OPTIBOOT_MINVER) "\n"
"  .section .text\n");

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "EEPROMDefs.h"
#include "SerialProtocol.h"
#include "uart.h"
#include "util.h"
#include "time_utils.h"
#include "twi.h"
#include "mfi.h"
#include "boot.h"
#include "ringbuffer.h"
#include <util/setbaud.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "mfiboot.h"


//
// Global Variables - most variables are global for code size
//
//#define certDataBytesLeft (*((uint16_t *)&GPIOR1))
register uint8_t myIndex asm("r3");
register uint8_t outIndex asm("r4");
register uint8_t inIndex asm("r5");
register uint8_t trigger asm("r6");
register uint8_t intBufUsed asm("r7");
register uint8_t intBufIndex asm("r8");


uint8_t data[CMD_MAX_PAYLOAD];
uint16_t address;
//uint16_t adcValue;
uint16_t previousADCValue;
uint8_t robotType;
volatile uint16_t intCnt0;

uint8_t intBuf[64];
uint8_t bytesInSection;
uint8_t certPages;
uint16_t certDataBytesLeft;

//volatile Ringbuf outBuf;
volatile Ringbuf inBuf;

//volatile bootloaderStatus_t bootloaderStatus;
iAPIDPSStatus_t iAPIDPSStatus;



// C zero initialises all global variables. However, that requires
// These definitions are NOT zero initialised, but that doesn't matter
// This allows us to drop the zero init code, saving us memory
#define buff    ((uint8_t*)(RAMSTART))


// Timer interrupt at around 30kHz @8MHz, 46.875kHz @12MHz, 60kHz @16MHz
ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
    intCnt0++;
    trigger++;
    
    if(trigger > 10) {
        loopTriggered = TRUE;
        trigger = 0;
    }
    
    // If we have bytes saved in the interrupt buffer, these should be transferred
    // to the input buffer first
    #ifdef USE_MUTEX
    if(intBufUsed) {
        if(MutexTryLockInBuf()) {
            for(intBufIndex=0; intBufIndex < intBufUsed; intBufIndex++) {
                PutByteInBuf(intBuf[intBufIndex]);
            }
            MutexUnlockInBuf();
            intBufUsed = 0;
        }
    }
    #endif
    
    // If we have received a new byte on the serial port, attempt to put it in the
    // input buffer, but if that fails, put it in the interrupt buffer and we'll
    // come back to that later
    if(UART1_RX_COMPLETE) {
        #ifdef USE_MUTEX
        if(MutexTryLockInBuf()) {
            PutByteInBuf(UDR1);
            MutexUnlockInBuf();
        }
        else {
            if(intBufUsed < 64) {
                intBuf[intBufUsed++] = UDR1;
            }
        }
        #else
        PutByteInBuf(UDR1);
        #endif //USE_MUTEX
    }
}



//
// main program starts here
//
int main(void) {
    // Making these local and in registers prevents the need for initializing
    // them, and also saves space because code no longer stores to memory.
    // (initializing address keeps the compiler happy, but isn't really
    // necessary, and uses 4 bytes of flash.)
    char tmp[20];
    register uint8_t ch;
    register uint8_t length;
    myIndex = 0;
    outIndex = 0;
    inIndex = 0;
    trigger = 0;
    sessionID = 1;
    transIDRomo = 0xFFFF;
    GPIOR0 = 0;
    inBufUsed = 0; // reset inBufUsed which is stored in GPIOR1
    GPIOR2 = 0;
    
    #ifdef USE_MUTEX
    intBufUsed = 0;
    intBufIndex = 0;
    #endif
    
    // Setup the various registers
    MCUSR = 0; // Clear any reset flags
    
    MCUCR = (1<<IVCE); // enable moving vector table
    MCUCR = (1<<IVSEL); // move vector table to bootloader section

    TCCR0B = 0x01; // setup timer0 with no prescalar, normal mode
    TIMSK0 = 0x01; // enable timer0 overflow interrupt
    
    DDRB = 0x19;  // set motor control lines as outputs
    DDRD = 0xFA;  // set motor control lines as outputs
    DDRC = 0x40;  // set !CPRESET as output
    PORTC = 0x80; // enable pullup on /CHARGING_LED
    DIDR0 = 0xFF; // disable digital input buffers on analog pins

    // Bring !CPRESET low to start a coprocessor reset
    cbi(PORTC, PORTC6);
    
    _delay_us(15);
    
    // Return !CPRESET to high between 10us and 1ms after falling edge
    sbi(PORTC, PORTC6);
    
    // Store the bootloader version in eeprom if it's different
    length = eeprom_read_byte(EEPROM_BOOTLOADER_MAJOR_VERSION_ADDRESS);
    if(length != OPTIBOOT_MAJVER) {
        eeprom_write_byte(EEPROM_BOOTLOADER_MAJOR_VERSION_ADDRESS, OPTIBOOT_MAJVER);
    }
    length = eeprom_read_byte(EEPROM_BOOTLOADER_MINOR_VERSION_ADDRESS);
    if(length != OPTIBOOT_MINVER) {
        eeprom_write_byte(EEPROM_BOOTLOADER_MINOR_VERSION_ADDRESS, OPTIBOOT_MINVER);
    }
    length = 0;
    
    wdt_enable(WATCHDOG_500MS);

    //
    // Special modes and robot type detection
    //
    #if 1
    // This will get us right in to BoN mode (20 bytes)
    if(bit_is_set(PINB, PINB1)) {
        appStart();
    }

    // Read the voltage on DEVICEDET
    adcWaitForConversion(PINA7);
    
    // Any value other than ~0 on DEVICEDET indicates a special mode, so go straight to main firmware
    if(ADCW > LOW_VOLTAGE_THRESHOLD) {
        appStart();
    }

    // Read the voltage on ACCPWR
    adcWaitForConversion(PINA0);

    // Any voltage on ACCPWR other than ~3.3 means it's a lightning robot in a special mode
    if(ADCW < HIGH_VOLTAGE_THRESHOLD) {
        appStart();
    }

    // Now test to see if we are lightning or 30-pin
    sbi(PORTA, PINA5); // Turn on pullup for USBD+
    adcWaitForConversion(PINA6);
    
    previousADCValue = ADCW;
    
    cbi(PORTA, PINA5); // Turn off USBD+ pullup
    sbi(DDRA, PINA5); // Set USBD+ to 0
    adcWaitForConversion(PINA6);

    if((int16_t)(previousADCValue - ADCW) > 74) {
        robotType = 1;
    }
    else {
        robotType = 0;
    }
    #else
    robotType = 1;    
    #endif
    
    cbi(PORTA, PINA5); // Restore USBD+ to 0
    cbi(PORTA, PINA6); // Turn off pullup on USBD-
    cbi(DDRA, PINA5); // Restore USBD+ as an input

    sei();
    
    // UART Init
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
    #if USE_2X
    UCSR1A |= (1 << U2X1);
    #else
    UCSR1A &= ~(1 << U2X1);
    #endif

    UCSR1B = _BV(RXEN1) | _BV(TXEN1); // Enable Rx, Tx
    UCSR1C = _BV(UCSZ00) | _BV(UCSZ01); // Set character size to 8-bit

    #if 0
    DDRB = 0x1F;
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    UCSR0A |= (1 << U2X0);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0); // Enable Rx, Tx
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // Set character size to 8-bit

    UART0_TX_WHEN_READY(0xAA);
    #endif
        
    // TWI Init
    twiState = TWI_NO_STATE;
    TWI_SET_FREQUENCY(IPOD_CP_FREQUENCY_LOW); // Initialize to low speed
    TWDR = 0xFF; // Default content = SDA released.
    TWCR =	(1<<TWEN)| // Enable TWI-interface and release TWI pins.
    (0<<TWIE)|(0<<TWINT)| // Disable Interrupt.
    (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)| // No Signal requests.
    (0<<TWWC);
    
    
    //
    // The main bootloader loop
    //
    while(1) {
        if(loopTriggered) {
            loopTriggered = FALSE;
            
            // Read the voltage on the charging LED to see if we are hooked to a charger
            if(bit_is_clear(PINC, PINC7)) {
                appStart();
            }
            
            // Send the first iAP sync byte at least 80ms after powerup, and the second
            // 20ms after that. The initialization before the loop takes about 70ms
            if(!sentSyncs) {
                if(!myIndex) {
                    intCnt0 = 0;
                    myIndex++;
                }
                if(intCnt0 >= MS_TO_TICKS(20)) {
                    intCnt0 = 0;
                    putch(0xFF);
                    myIndex++;
                }
                if(myIndex == 3) {
                    sentSyncs = TRUE;
                }
            }
            
            // Flash the LEDs to indicate we are in the bootloader main loop
            // FIXME: This can be optimized I'm sure - aarons
            if(!flashedLEDs) {
                if(bit_is_clear(LED_PORT, LED)) {
                    sbi(LED_PORT, LED);
                    intCnt0 = 0;
                }
                else if(intCnt0 >= MS_TO_TICKS(50)) {
                    cbi(LED_PORT, LED);
                    flashedLEDs = TRUE;
                }
            }
            
            //
            // IDPS processing
            //
            #if 1
            if(usingiAP) {
                MFIGetCommand();
                if(newCommand) {
                    #if 1
                    if(mfiCommand == IPOD_iPodAck) {
                        newCommand = FALSE; // ACks don't count as commands
                        switch(pCommandData[1]) {
                            #if 1
                            case IPOD_StartIDPS:
                            if(!iAPIDPSStatus.maxPayloadSizeRequested) {
                                MFISendCommand(IPOD_RequestTransportMaxPayloadSize, 0);
                                iAPIDPSStatus.maxPayloadSizeRequested = TRUE;
                            }
                            break;
                            
                            case IPOD_RetAccessoryAuthenticationInfo:
                            certDataBytesLeft -= bytesInSection;
                            myIndex++;
                            
                            case IPOD_AccessoryDataTransfer:
                            case IPOD_RequestApplicationLaunch:
                            case IPOD_SetInternalBatteryChargingState:
                            case IPOD_RequestTransportMaxPayloadSize:
                            case IPOD_GetiPodOptionsForLingo:
                            case IPOD_SetFIDTokenValues:
                            default:
                            waitingForAck = FALSE;
                            break;
                            #endif
                        }
                    }
                    
                    else if(mfiCommand == IPOD_iPodDataTransfer) {
                        sessionID = pCommandData[1];
                        switch((RMCommandToRobot)pCommandData[2]) {
                            case RMCommandToRobotSTKEnterProgrammingMode:
                            case RMCommandToRobotSTKReadSignature:
                            programming = TRUE;
                            
                            case RMCommandToRobotSTKGetParameter:
                            case RMCommandToRobotSTKSetDevice:
                            case RMCommandToRobotSTKSetDeviceExt:
                            case RMCommandToRobotSTKLoadAddress:
                            case RMCommandToRobotSTKUniversal:
                            case RMCommandToRobotSTKProgramPage:
                            case RMCommandToRobotSTKReadPage:
                            inIndex = 2;
                            outIndex = 4;
                            newCommand = TRUE;
                            break;

                            // Jump to the main firmware if asked
                            case RMCommandToRobotSTKLeaveProgrammingMode:
                            default:

                            MFISendAck(mfiCommand, 0);
                            appStart();
                            break;
                        }
                        
                        MFISendAck(mfiCommand, 0);
                    }
                    
                    else if(mfiCommand == IPOD_ReturnTransportMaxPayloadSize) {
                        if(!iAPIDPSStatus.optionsForLingoRequested) {
                            data[0] = IPOD_LINGO_GENERAL;
                            MFISendCommand(IPOD_GetiPodOptionsForLingo, 1);
                            iAPIDPSStatus.optionsForLingoRequested = TRUE;
                        }
                    }
                    
                    else if(mfiCommand == IPOD_RetiPodOptionsForLingo) {
                        // <Lingo ID> <Option Bits byte 7> ... <Options Bits byte 0>
                        // ensure device supports communication with app (bit 13) and request application launch (bit 24)
                        wdt_disable();
                        
                        // Device does not support communication, so end IDPS
                        if(bit_is_clear(pCommandData[7], 5)) { // && //bit_is_set(pCommandData[5], 0) &&
                            data[0] = 2; // Abandon IDPS process
                            MFISendCommand(IPOD_EndIDPS, 1);
                        }

                        else if(!iAPIDPSStatus.FIDTokensSent) {
                            //SetFIDTokenValues
                            data[0] = 0; // first byte is number of tokens, incremented by Add*Token functions
                            myIndex = 1; // start sticking data at position 1
                            memset(tmp, 0, 10);

                            tmp[0] = 1;
                            tmp[5] = 2;
                            CPRead(AUTH_DEVICE_ID, (uint8_t *)tmp+6, 4);
                            AddFIDToken(FID_IDENTIFY_TOKEN, tmp, 12);

                            memset(tmp, 0, 10);
                            tmp[6] = 2; //9th bit of capabilities bitmask set
                            AddFIDToken(FID_ACCESSORY_CAPS_TOKEN, tmp, 10);
                            AddFIDInfoToken(FID_ACC_INFO_NAME, "Romo", 8);
                            tmp[1] = OPTIBOOT_MAJVER;
                            tmp[2] = OPTIBOOT_MINVER;
                            AddFIDInfoToken(FID_ACC_INFO_FIRMWARE_VERSION, tmp, 6);

                            tmp[0] = eeprom_read_byte(EEPROM_HW_MAJOR_VERSION_ADDRESS);
                            tmp[1] = eeprom_read_byte(EEPROM_HW_MINOR_VERSION_ADDRESS);
                            tmp[2] = eeprom_read_byte(EEPROM_HW_REVISION_VERSION_ADDRESS);

                            AddFIDInfoToken(FID_ACC_INFO_HARDWARE_VERSION, tmp, 6);
                            AddFIDInfoToken(FID_ACC_INFO_MANUFACTURER, "Romotive, Inc", 17);
                            if(robotType) {
                                AddFIDInfoToken(FID_ACC_INFO_MODEL_NUMBER, "3l", 6);
                            }
                            else {
                                AddFIDInfoToken(FID_ACC_INFO_MODEL_NUMBER, "3b", 6);
                            }

                            // Read EEPROM data
                            ReadSerialNumber((char *)&tmp);

                            AddFIDInfoToken(FID_ACC_INFO_SERIAL_NUMBER,  tmp, 24);

                            tmp[0] = 0;
                            tmp[1] = CMD_MAX_PAYLOAD;
                            AddFIDInfoToken(FID_ACC_INFO_MAX_PAYLOAD_SIZE, tmp, 5);
                            memset(tmp, 0, 10);
                            AddFIDInfoToken(FID_ACC_INFO_RF_CERTIFICATIONS, tmp, 7);
                            AddFIDTokenWithByte(FID_EA_PROTOCOL_TOKEN, IPOD_ACCESSORY_EA_INDEX, IPOD_ACCESSORY_EA_STRING, 21);
                            AddFIDToken(FID_BUNDLE_SEED_ID_PREF_TOKEN, IPOD_ACCESSORY_BUNDLE_SEED_ID, 13);

                            tmp[0] = 1;
                            tmp[1] = 1;
                            AddFIDToken(FID_EA_PROTOCOL_METADATA_TOKEN, tmp, 4);

                            MFISendCommand(IPOD_SetFIDTokenValues, myIndex);
                            iAPIDPSStatus.FIDTokensSent = TRUE;
                        }
                    }
                    
                    else if(mfiCommand == IPOD_IDPSStatus) {
                        if(pCommandData[0]) {
                            // Send IdentifyDeviceLingoes with all parameters set to 0xFF to trigger an
                            // "Accessory not supported" message
                            transIDRomo = 0xFFFF;
                            memset(data, 0xFF, 10);
                            MFISendCommand(IPOD_IdentifyDeviceLingoes, 10);
                        }
                    }
                    
                    else if(mfiCommand == IPOD_AckFIDTokenValues) {
                        if(!iAPIDPSStatus.endIDPSRequested) {
                            data[0] = 0; // Finished IDPS, move to authentication
                            MFISendCommand(IPOD_EndIDPS, 1);
                            iAPIDPSStatus.endIDPSRequested = TRUE;
                        }
                    }
                    
                    else if(mfiCommand == IPOD_GetAccessoryAuthenticationInfo) {
                        iAPIDPSStatus.authenticationInfoRequested = TRUE;
                        waitingForAck = FALSE;
                        myIndex = 0;
                        
                        // Read the length of the authentication certificate
                        CPRead(AUTH_ACC_CERT_DATA_LENGTH, (uint8_t *)&certDataBytesLeft, 2);
                        
                        //flip the bytes around
                        ch = certDataBytesLeft;
                        certDataBytesLeft = certDataBytesLeft>>8;
                        certDataBytesLeft = (uint16_t)(ch<<8) | certDataBytesLeft;

                        certPages = certDataBytesLeft/(uint8_t)MFI_AUTH_PAGE_SIZE;

                        if(!(certDataBytesLeft % (uint8_t)MFI_AUTH_PAGE_SIZE)) {
                            certPages--;	// lastSection is 0-based, so catch exact page size case here
                        }
                    }
                    
                    //case IPOD_AckAccessoryAuthenticationInfo:
                    else if(mfiCommand == IPOD_AckAccessoryAuthenticationInfo) {
                        iAPIDPSStatus.authenticationInfoAcked = TRUE;
                    }
                    
                    else if(mfiCommand == IPOD_GetAccessoryAuthenticationSignature) {
                        // Skipping sending length to co-processor since it seems to work without it
                        // Send challenge data to co-processor
                        CPWrite(AUTH_CHALLENGE_DATA, pCommandData, 20);

                        // Start signature generation
                        data[0] = 1;
                        CPWrite(AUTH_CONTROL_STATUS, data, 1);

                        // Theoretically we would check this result, but we don't have the space
                        //            CPRead(AUTH_CONTROL_STATUS, data, 1);

                        // Read back signature
                        CPRead(AUTH_SIGNATURE_DATA, data, 128);

                        // Send signature data to the iDevice
                        MFISendAuthData(IPOD_RetAccessoryAuthenticationSignature, 128);
                    }
                    
                    else if(mfiCommand == IPOD_AckAccessoryAuthenticationStatus) {
                        if(pCommandData[0] == 0) {
                            applicationLaunchRequested = TRUE;
                        }
                        // If authentication status was some sort of failure, restart
                        else {
                            reset();
                        }
                        
                    }
                    
                    else if(mfiCommand == IPOD_OpenDataSessionForProtocol) {
                        //sessionID = (pCommandData[0] << 8) | pCommandData[1];
                        sessionID = pCommandData[1]; // assume that we don't have a sessionID > 255
                        MFISendAck(IPOD_OpenDataSessionForProtocol, 0);
                    }
                    #endif
                }
            }
            #endif
            
            // Determine if we are speaking iAP or STK500 protocols
            #if 1
            if(!usingiAP && !programming) {
                while(BytesUsedInBuf() > 1) {
                    ch = GetByteInBuf();
                    // Check if we have an iAP SYNC+START
                    if(ch == SYNC_BYTE) {
                        if(PeekByte(&inBuf, 0) == START_BYTE) {
                            usingiAP = TRUE;
                            break;
                        }
                    }
                    
                    // Check if we have STK500 SYNC+CRCEOP
                    else if(ch == RMCommandToRobotSTKGetSync) {
                        if(PeekByte(&inBuf, 0) == RMCommandToRobotSTKCRCEOP) {
                            programming = TRUE;
                            GetByteInBuf(); // Remove the STK Sync byte
                            break;
                        }
                    }
                }
            }
            
            if(!programming && sentSyncs && !iAPIDPSStatus.maxPayloadSizeRequested &&
            ((intCnt0 >= MS_TO_TICKS(500)) || !iAPIDPSStatus.startIDPSRequested)) {
                // Send start IDPS every 500ms
                // ?? Do we want to reset the transaction ID each time here?
                MFISendCommand(IPOD_StartIDPS, 0);
                iAPIDPSStatus.startIDPSRequested = TRUE;
                intCnt0 = 0;
                myIndex=1;
            }

            #endif
            
            // Send authentication certificate
            if(iAPIDPSStatus.authenticationInfoRequested &&
            !iAPIDPSStatus.authenticationInfoAcked &&
            !waitingForAck) {
                // Packet data
                data[0] = 0x02; // protocolMajorVersion;
                data[1] = 0x00; // protocolMinorVersion;
                data[2] = myIndex; // sectionIndex;
                data[3] = certPages; // lastSection;
                
                // Get certificate data from the co-processor
                bytesInSection = (certDataBytesLeft > MFI_AUTH_PAGE_SIZE) ? MFI_AUTH_PAGE_SIZE : certDataBytesLeft;
                CPRead(AUTH_ACC_CERT_DATA_START + myIndex, data+4, bytesInSection);
                
                // Send certificate data to the iDevice
                MFISendAuthData(IPOD_RetAccessoryAuthenticationInfo, bytesInSection+4);
                waitingForAck = TRUE;
            }
            
            // Request that our application is launched. Only do this once
            if(applicationLaunchRequested) {
                // Request that our app is loaded
                data[0] = 0; //reserved
                data[1] = 2; //First attempt
                data[2] = 0; //reserved
                memcpy(data+3, IPOD_ACCESSORY_BUNDLE_ID, 19);
                MFISendCommand(IPOD_RequestApplicationLaunch, 21);
                applicationLaunchRequested = FALSE;
            }
            
            //
            // programming routine
            //
            #if 1
            if(programming) {
                if(usingiAP && (waitingForAck || !newCommand)) {
                    continue;
                }
                // if we're not using iAP, interrupts are staying off for the duration, so force the loop to spin
                if(!usingiAP) {
                    cli(); // no interrupts during programming
                    loopTriggered = TRUE;
                }
                // get character from UART
                ch = getByte();

                if(ch == RMCommandToRobotSTKGetParameter) {
                    unsigned char which = getByte();
                    verifySpace();
                    if(which == 0x82) {
                        // Send optiboot version as "minor SW version"
                        putByte(OPTIBOOT_MINVER);
                    }
                    else if(which == 0x81) {
                        putByte(OPTIBOOT_MAJVER);
                    }
                    else {
                        // GET PARAMETER returns a generic 0x03 reply for
                        // other parameters - enough to keep Avrdude happy
                        putByte(0x03);
                    }
                }
                else if(ch == RMCommandToRobotSTKSetDevice) {
                    // SET DEVICE is ignored
                    getNBytes(20);
                }
                else if(ch == RMCommandToRobotSTKSetDeviceExt) {
                    // SET DEVICE EXT is ignored
                    getNBytes(5);
                }
                else if(ch == RMCommandToRobotSTKLoadAddress) {
                    // LOAD ADDRESS
                    uint16_t newAddress;
                    newAddress = getByte();
                    newAddress = (newAddress) | (getByte() << 8);

                    verifySpace();
                    newAddress += newAddress; // Convert from word address to byte address
                    address = newAddress;
                    
                }
                else if(ch == RMCommandToRobotSTKUniversal) {
                    // UNIVERSAL command is ignored
                    getNBytes(4);
                    putByte(0x00);
                }
                else if(ch == RMCommandToRobotSTKProgramPage) {   // Write memory, length is big endian and is in bytes
                    // PROGRAM PAGE - we support flash programming only, not EEPROM
                    uint8_t *bufPtr;
                    uint16_t addrPtr;

                    getByte();
                    length = getByte();
                    getByte();

                    cli();

                    // If we are in RWW section, immediately start page erase
                    if(address < NRWWSTART) {
                        __boot_page_erase_short((uint16_t)(void*)address);
                    }
                    
                    // While that is going on, read in page contents
                    bufPtr = buff;
                    do *bufPtr++ = getByte();
                    while(--length);

                    // If we are in NRWW section, page erase has to be delayed until now.
                    // Todo: Take RAMPZ into account
                    if(address >= NRWWSTART) {
                        __boot_page_erase_short((uint16_t)(void*)address);
                    }
                    
                    // Read command terminator, start reply
                    verifySpace();

                    // If only a partial page is to be programmed, the erase might not be complete.
                    // So check that here
                    boot_spm_busy_wait();

                    // Copy buffer into programming buffer
                    bufPtr = buff;
                    addrPtr = (uint16_t)(void*)address;
                    ch = SPM_PAGESIZE / 2;
                    do {
                        uint16_t a;
                        a = *bufPtr++;
                        a |= (*bufPtr++) << 8;
                        __boot_page_fill_short((uint16_t)(void*)addrPtr,a);
                        addrPtr += 2;
                    } while(--ch);

                    // Write from programming buffer
                    __boot_page_write_short((uint16_t)(void*)address);
                    boot_spm_busy_wait();

                    // Reenable read access to flash
                    boot_rww_enable();
                    sei();
                }
                else if(ch == RMCommandToRobotSTKReadPage) { // Read memory block mode, length is big endian.
                    // READ PAGE - we only read flash
                    getByte();
                    length = getByte();
                    getByte();
                    verifySpace();
                    cli();
                    do {
                        putByte(pgm_read_byte_near(address++));
                    } while(--length);
                    sei();
                }
                else if(ch == RMCommandToRobotSTKReadSignature) { // Get device signature bytes
                    // READ SIGN - return what Avrdude wants to hear
                    verifySpace();
                    putByte(SIGNATURE_0);
                    putByte(SIGNATURE_1);
                    putByte(SIGNATURE_2);
                }
                else {
                    // This covers the response to commands like STK_ENTER_PROGMODE
                    verifySpace();
                }
                putByte(RMCommandToRobotSTKOK);

                if(usingiAP) {
                    //data[0] = (sessionID>>8);
                    data[0] = 0; // Assuming we don't have a sessionID > 255
                    data[1] = sessionID;
                    data[2] = CMD_ACK;
                    data[3] = outIndex-4;
                    MFISendCommand(IPOD_AccessoryDataTransfer, outIndex);
                }
                
                if(ch == RMCommandToRobotSTKLeaveProgrammingMode) {
                    appStart();
                }
            }
            #endif
        }
    }
}


void putch(uint8_t ch)
{
    UART1_TX_WHEN_READY(ch);
}


void putByte(uint8_t ch)
{
    usingiAP ? data[outIndex++] = ch : putch(ch);
}


uint8_t getch()
{
    while(!(UCSR1A & _BV(RXC1)));
    wdt_reset();
    return UDR1;
}


uint8_t getByte()
{
    return usingiAP ? pCommandData[inIndex++] : getch();
}


void getNBytes(uint8_t count)
{
    do getByte(); while(--count);
    verifySpace();
}


void verifySpace()
{
    if(getByte() != RMCommandToRobotSTKCRCEOP) {
        reset();
    }
    putByte(RMCommandToRobotSTKInSync);
}


void adcWaitForConversion(uint8_t ch)
{
    _delay_ms(10);
    ADMUX = ch;
    ADCSRA = 0xC7; //enable, start conversion, and clock div factor 128
    while(bit_is_clear(ADCSRA, ADIF));
    _delay_ms(5);
}


void reset()
{
    wdt_enable(WATCHDOG_16MS);
    while(1);
}


void appStart()
{
    // Set interrupt vector table to application section
    MCUCR = (1<<IVCE);
    MCUCR = 0;
    
    // Save relevant iAP bits for main app
    GPIOR0 = sessionID;
    GPIOR1 = (transIDRomo>>8); // store this to send to the main app
    GPIOR2 = transIDRomo;

    __asm__ __volatile__ (
    "jmp 0000"
    );
}


void ReadSerialNumber(char *serial)
{
    char buf1[20];
    char buf2[20];
    char buf3[20];
    int i=19;

    // Read EEPROM data into buffers
    eeprom_read_block((void *)&buf1, EEPROM_SERIALNUMBER_ADDRESS, 20);
    eeprom_read_block((void *)&buf2, EEPROM_SERIALNUMBER_ADDRESS_ALT1, 20);
    eeprom_read_block((void *)&buf3, EEPROM_SERIALNUMBER_ADDRESS_ALT2, 20);

    do {
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
    } while(i--);
    
    serial[19] = 0;

    return;
}


