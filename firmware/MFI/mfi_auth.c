/*
* mfi_auth.c
*
* Created: 8/20/2012 7:29:54 PM
*  Author: Dan Kane
*/

#include "main.h"
#include "mfi_auth.h"
#include "mfi.h"
#include "twi.h"
#include "uart.h"
#include "timers.h"
#include <string.h>
#include <util/atomic.h>


struct mfiAuthStatus_t mfiAuthStatus;


BOOL MFIAuthInit()
{
    mfiAuthStatus.protocolMajorVersion = 0;
    mfiAuthStatus.protocolMinorVersion = 0;
    mfiAuthStatus.acked.all = 0;
    
    CPInit();								// Initialize the co-processor (sets I2C baud)
    
    // Run CP self test
    uint8_t selfTestData = 0x01;
    CPWrite(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);	// Send 1 to CP self test register to initiate
    CPRead(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);		// Read results (bits 7-4 hold the result)
    
    if(bit_is_clear(selfTestData, CP_SELFTEST_X509_BIT) ||      // Bit 7 = x.509 certificate: 0=not found, 1=found
    bit_is_clear(selfTestData, CP_SELFTEST_PK_BIT)) {           // Bit 6 = private key: 0=not found, 1=found
        return FALSE;
    }
    
    // CP self test passed!
    
    // Get Authentication Version from co-processor
    CPRead(AUTH_PROTOCOL_MAJOR_VERSION, &mfiAuthStatus.protocolMajorVersion, 1);
    CPRead(AUTH_PROTOCOL_MINOR_VERSION, &mfiAuthStatus.protocolMinorVersion, 1);
    
    return TRUE;
}

void MFIAuthTasks()
{
    uint8_t data[IPOD_COMMAND_PAYLOAD_SIZE];
    
    if (mfiStatus.flags.needsAuthentication) {
        mfiStatus.communication.isAuthenticating = TRUE;
        mfiStatus.flags.needsAuthentication = FALSE;
        mfiAuthStatus.acked.all = 0;
        
        // Get certificate data length from co-processor
        uint16_t certDataLength;
        CPRead(AUTH_ACC_CERT_DATA_LENGTH, (uint8_t *)&certDataLength, 2);
        certDataLength = (certDataLength<<8) + (certDataLength>>8);             // certDataLength appears to come back flipped
        // Calculate the maximum number of sections that will be sent
        mfiAuthStatus.lastSection = certDataLength/(uint8_t)MFI_AUTH_PAGE_SIZE;
        if(!(certDataLength % (uint8_t)MFI_AUTH_PAGE_SIZE)) {
            mfiAuthStatus.lastSection--;	// lastSection is 0-based, so catch exact page size case here
        }
        mfiAuthStatus.certDataBytesLeft = certDataLength;
        // (Re)Set section index
        mfiAuthStatus.sectionIndex = 0;
    }
    
    if(mfiStatus.communication.isAuthenticating) {
        if(!mfiAuthStatus.acked.retAccessoryAuthenticationInfo) {
            // Packet data
            data[0] = mfiAuthStatus.protocolMajorVersion;
            data[1] = mfiAuthStatus.protocolMinorVersion;
            data[2] = mfiAuthStatus.sectionIndex;
            data[3] = mfiAuthStatus.lastSection;
            
            // Get certificate data from the co-processor
            uint8_t bytesInSection = (mfiAuthStatus.certDataBytesLeft > MFI_AUTH_PAGE_SIZE) ? MFI_AUTH_PAGE_SIZE : mfiAuthStatus.certDataBytesLeft;
            CPRead(AUTH_ACC_CERT_DATA_START + mfiAuthStatus.sectionIndex, data+4, bytesInSection);
            
            // Send certificate data to the iDevice
            mfiAuthStatus.transIDs.retAccessoryAuthenticationInfo = mfiStatus.transactionIDiPod;
            MFISendCommand(IPOD_RetAccessoryAuthenticationInfo, data, bytesInSection+4, mfiStatus.transactionIDiPod);

            mfiAuthStatus.certDataBytesLeft -= bytesInSection;
            
            // Set ack so that we don't try again until the ack is received (and section is incremented)
            mfiAuthStatus.acked.retAccessoryAuthenticationInfo = TRUE;
        }
    }
}


void MFIAuthSendSignature(uint8_t *challenge, uint16_t length)
{
    // Length should be 20, but they want 2 bytes for it on the CP
    
    uint8_t data[IPOD_COMMAND_PAYLOAD_SIZE];
    uint16_t sigDataLen = 0;	// Should be 128 bytes, read from CP later
    
    // Send challenge data length to co-processor
    uint16_t lengthFlipped = (length>>8) + (length<<8);
    CPWrite(AUTH_CHALLENGE_DATA_LENGTH, (uint8_t *)&lengthFlipped, 2);
    // Send challenge data to co-processor
    CPWrite(AUTH_CHALLENGE_DATA, challenge, (uint8_t)length);
    
    // Start signature generation
    data[0] = 1;
    CPWrite(AUTH_CONTROL_STATUS, data, 1);
    
    // Check for result
    CPRead(AUTH_CONTROL_STATUS, data, 1);
    if(BitIsSet(data[0], 7)) {  // Bit 7 is the error bit
        // Error encountered.  Read error register to determine what happened (clears error state)
        CPRead(AUTH_ERROR_CODE, data, 1);
    }
    else if(((data[0] & CP_STATUS_BITMASK)>>4) == 1) { // Mask unwanted bits and shift to start of byte
        // Read back signature
        CPRead(AUTH_SIGNATURE_DATA_LENGTH, (uint8_t *)&sigDataLen, 2);
        sigDataLen = (sigDataLen>>8) + (sigDataLen<<8);
        CPRead(AUTH_SIGNATURE_DATA, data, sigDataLen);
        
        // Send signature data to the iDevice
        mfiAuthStatus.transIDs.retAccessoryAuthenticationSignature = mfiStatus.transactionIDiPod;
        MFISendCommand(IPOD_RetAccessoryAuthenticationSignature, data, sigDataLen, mfiStatus.transactionIDiPod);
    }
    else {
        data[0] = 0;    //TESTING
    }
}

//////////////////////////////////////////////////////////////////////////
// Apple Authentication Co-processor Functions
//////////////////////////////////////////////////////////////////////////

void CPInit()
{
    /* *
    * The 2.0B co-processor runs at 40kHz, while the 2.0C can run at 400kHz.
    * This function will initialize to the lower speed, check the co-processor version
    * and reset the speed if necessary
    * */
    
    sbi(PORTC, PORTC6);											// Set CPRESET high to configure address
    
    TWIInit();													// Initialize first to low speed
    delay_ms(10);									            // First data transmission must be >10ms after CPRESET goes high

    uint8_t deviceVersion = 0;
    CPRead(AUTH_DEVICE_VERSION, &deviceVersion, 1);				// Read Device Version from the CP
    if(deviceVersion == IPOD_CP_VERSION_2_0C) {                 // Set to high speed if it can handle it
        TWI_SET_FREQUENCY(IPOD_CP_FREQUENCY_HIGH);
    }
    return;
}


BOOL CPWrite(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
    // Temporarily Disable UART
    #ifdef USE_UART1_RECEIVE_INTERRUPT
    UART1_RX_DISABLE;
    #endif
    #ifdef USE_UART1_TRANSMIT_INTERRUPT
    UART1_TX_DISABLE;
    #endif
    
    uint8_t msg[dataSize+2];
    msg[0] = IPOD_CP_INTERFACE & ~_BV(TWI_READ_BIT);			// First byte is the CP slave write address
    msg[1] = addr;		    									// Second byte is register address at which to begin writing
    if(dataSize > 0) {
        memcpy(msg+2, data, dataSize);							// Copy data from data to msg starting at [2]
    }
    TWIStartTransceiverWithData(msg,dataSize+2);				// Send start and write address with data
    uint8_t twiState = TWIGetStateInfo();						// Get state to check for NACK
    intCnt0 = 0;
    while(twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK) {
        if(intCnt0 <= MS_TO_TICKS(1000)) {     // Try for ~1s
            delay_ms(100);						// Wait
            TWIStartTransceiver();				// Try again
            twiState = TWIGetStateInfo();		// Get new state
        }
        else {
            return FALSE;
        }
    }
    
    // Re-enable UART
    #ifdef USE_UART1_RECEIVE_INTERRUPT
    UART1_RX_ENABLE;
    #endif
    #ifdef USE_UART1_TRANSMIT_INTERRUPT
    UART1_TX_ENABLE;
    #endif

    return TRUE;
}

BOOL CPRead(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
    CPWrite(addr, data, 0);										// First send write sequence with no data
    
    // Temporarily Disable UART
    #ifdef USE_UART1_RECEIVE_INTERRUPT
    UART1_RX_DISABLE;
    #endif
    #ifdef USE_UART1_TRANSMIT_INTERRUPT
    UART1_TX_DISABLE;
    #endif
    
    uint8_t msg[dataSize+1];									// Include one extra byte for the address
    msg[0] = IPOD_CP_INTERFACE | _BV(TWI_READ_BIT);				// Set the CP slave read address
    TWIStartTransceiverWithData(msg,dataSize+1);				// Send start and read address
    uint8_t twiState = TWIGetStateInfo();						// Get state to check for address NACK
    intCnt0 = 0;
    while(twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK) {
        if(intCnt0 <= MS_TO_TICKS(1000)) {      // Try for ~1s
            delay_ms(100);						// Wait
            TWIStartTransceiver();				// Try again
            twiState = TWIGetStateInfo();		// Get new state
        }
        else {
            return FALSE;
        }
    }
    
    if(!TWIGetDataFromTransceiver(msg, dataSize+1)) {       // Get data from the buffer
        return FALSE;										// Fail if unsuccessful
    }
    
    if(msg[0] == (IPOD_CP_INTERFACE | _BV(TWI_READ_BIT))) { // Check if the message is intended for us
        //memcpy(data,msg+1, dataSize);						// Copy data from msg to data, skipping the address byte
        // Bytes appear to need to be flipped
        for(uint8_t i=0; i<dataSize; i++) {
            //data[i] = msg[dataSize-i];                      // Copy data from msg to data, skipping the address byte
            data[i] = msg[i+1];                      // Copy data from msg to data, skipping the address byte
        }
    }
    else {
        data[0] = 0;
    }
    
    // Re-enable UART
    #ifdef USE_UART1_RECEIVE_INTERRUPT
    UART1_RX_ENABLE;
    #endif
    #ifdef USE_UART1_TRANSMIT_INTERRUPT
    UART1_TX_ENABLE;
    #endif

    return TRUE;
}