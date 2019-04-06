/*
* BoN_mode.c
*
* Created: 8/16/2012 11:18:17 PM
*  Author: Dan Kane
*/

#include "main.h"
#include "BoN_mode.h"
#include "eeprom.h"
#include "uart.h"
#include "util.h"
#include "mfi_auth.h"
#include "twi.h"
#include "timers.h"
#include "ringbuffer.h"
#include <avr/interrupt.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
// Bed of Nails Mode
//
// * Print status
//   * Firmware version
//   * Serial number
//   * Analog values
// * Receive new serial number
// * Send ack?
//////////////////////////////////////////////////////////////////////////

uint8_t cpSelfTestData;

/* *
* BoNModeInit()
*
* Disable all peripherals except the analog in and UART1
* Set all outputs to High Impedance (inputs)
* */
void BoNModeInit(void)
{
    // Initialize Co-processor
    sbi(PORTC, PORTC6);											// Set CPRESET high to configure address
    TWIInit();
    
    devFlags = 0x00; // clear all dev flags
    
    wdt_disable();
    
    GPIO->portB.direction.all = INPUT;
    GPIO->portC.direction.all = INPUT;
    GPIO->portD.direction.all = 0x0A; // All input except TX0 and TX1
}

void SendBoNStatus(void)
{
    #ifndef USE_HEX_VERSIONS
    char ver[4];
    #endif //!USE_HEX_VERSIONS
    
    // Send BoN Status data over UART
    
    // Greeting
    UART1TransmitCString("\nRomo 3 Bed of Nails Test\n");
    
    // Hardware Version
    UART1TransmitCString("Hardware Version: ");
    #ifdef USE_HEX_VERSIONS
    UART1TransmitUInt8(devInfo.hardwareVersion.major, FALSE);
    UART1_TX_WHEN_READY('.');
    UART1TransmitUInt8(devInfo.hardwareVersion.minor, FALSE);
    UART1_TX_WHEN_READY('.');
    UART1TransmitUInt8(devInfo.hardwareVersion.revision, FALSE);
    #else
    utoa(devInfo.hardwareVersion.major, ver, 10);
    UART1TransmitCString(ver);
    UART1_TX_WHEN_READY('.');
    utoa(devInfo.hardwareVersion.minor, ver, 10);
    UART1TransmitCString(ver);
    UART1_TX_WHEN_READY('.');
    utoa(devInfo.hardwareVersion.revision, ver, 10);
    UART1TransmitCString(ver);
    #endif
    UART1_TX_WHEN_READY('\n');

    
    // Firmware Version
    UART1TransmitCString("Firmware Version: ");
    #ifdef USE_HEX_VERSIONS
    UART1TransmitUInt8(FIRMWARE_VERSION_MAJOR, FALSE);
    UART1_TX_WHEN_READY('.');
    UART1TransmitUInt8(FIRMWARE_VERSION_MINOR, FALSE);
    UART1_TX_WHEN_READY('.');
    UART1TransmitUInt8(FIRMWARE_VERSION_REVISION, FALSE);
    #else
    utoa(FIRMWARE_VERSION_MAJOR, ver, 10);
    UART1TransmitCString(ver);
    UART1_TX_WHEN_READY('.');
    utoa(FIRMWARE_VERSION_MINOR, ver, 10);
    UART1TransmitCString(ver);
    UART1_TX_WHEN_READY('.');
    utoa(FIRMWARE_VERSION_REVISION, ver, 10);
    UART1TransmitCString(ver);
    #endif //USE_HEX_VERSIONS
    UART1_TX_WHEN_READY('\n');
    
    //Serial Number
    UART1TransmitCString("Serial Number: ");
    UART1TransmitCString(devInfo.serialNumber);
    UART1_TX_WHEN_READY('\n');
    
    // Co-processor Self Test Result
    UART1TransmitCString("CP Self Test Result:");
    if (BitIsSet(cpSelfTestData, CP_SELFTEST_X509_BIT) && BitIsSet(cpSelfTestData, CP_SELFTEST_PK_BIT)) {
        UART1TransmitCString(" Passed\n");
    } else {
        UART1TransmitCString(" Failed!\n");
        // Bit 7 = x.509 certificate: 0=not found, 1=found
        if (!BitIsSet(cpSelfTestData, CP_SELFTEST_X509_BIT)) {
            UART1TransmitCString("  * X.509 Certificate Not Found\n");
        }
        // Bit 6 = private key: 0=not found, 1=found
        if (!BitIsSet(cpSelfTestData, CP_SELFTEST_PK_BIT)) {
            UART1TransmitCString("  * Private Key Not Found\n");
        }
    }
    
    // Analog readings
    UART1TransmitCString("Analog Signals:\n");
    UART1TransmitCString("  +3.3V\t\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_ACCPWR], TRUE);
    UART1TransmitCString("\n  BATTMONITOR\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_BATTERY], TRUE);
    UART1TransmitCString("\n  M1CURRENT\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_M1CURRENT], TRUE);
    UART1TransmitCString("\n  M2CURRENT\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_M2CURRENT], TRUE);
    UART1TransmitCString("\n  M3CURRENT\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_M3CURRENT], TRUE);
    UART1TransmitCString("\n  USBD+\t\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_USBD_P], TRUE);
    UART1TransmitCString("\n  USBD-\t\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_USBD_N], TRUE);
    UART1TransmitCString("\n  DEVICEDET\t0x");
    UART1TransmitUInt16(devInfo.analogValues[ADC_DEVICEDET], TRUE);
    UART1_TX_WHEN_READY('\n');

}

/// Main BoN loop
void BoNMain(void)
{
    char serial[20];
    uint8_t i;
    BoNModeInit();
    
    // Run CP self test
    cpSelfTestData = 0x01;
    CPWrite(AUTH_SELFTEST_CONTROL_STATUS, &cpSelfTestData, 1);	// Send 1 to CP self test register to initiate
    CPRead(AUTH_SELFTEST_CONTROL_STATUS, &cpSelfTestData, 1);	// Read results (bits 7-4 hold the result)
    
    // Write result to eeprom
    eeprom_write_block((void *)&cpSelfTestData, EEPROM_CP_SELFTEST_ADDRESS, 1);
    
    SendBoNStatus();

    while(BytesUsedInBuf() < 19);

    
    // Read the 19 digits of the serial number from the inBuf
    GetBytesInBuf((uint8_t *)&serial, 19);

    serial[19] = '\0';

    WriteSerialNumber((char *)&serial);

    // Write new serial number to eeprom
    //eeprom_write_block((void *)&serial, EEPROM_SERIALNUMBER_ADDRESS, 20);
    
    // Read the serial number back to verify it
    //eeprom_read_block((void *)&devInfo.serialNumber, EEPROM_SERIALNUMBER_ADDRESS, 20);
    
    ReadSerialNumber((char *)&devInfo.serialNumber);
    
    
    for(i=0; i<20; i++) {
        if(devInfo.serialNumber[i] != serial[i]) {
            UART1TransmitCString("\nError writing serial number!");
            goto done;
        }
    }
    
    UART1TransmitCString("\nWrote serial number: "); // BoN fixture arduino depends on this string not changing
    UART1TransmitCString(devInfo.serialNumber);
    
    done:
    while(1);		// Done

}