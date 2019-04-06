/*
* mfi_auth.c
*
* Created: 8/20/2012 7:29:54 PM
*  Author: Dan Kane
*/


#include "mfi_auth.h"
#include "mfi_config.h"
#include "mfi.h"
#include "twi.h"
#include "commands.h"

#if 0

struct mfiAuthStatus_t mfiAuthStatus;

BOOL MFIAuthInit( void )
{
    mfiAuthStatus.protocolMajorVersion = 0;
    mfiAuthStatus.protocolMinorVersion = 0;
    mfiAuthStatus.acked.all = 0;
    
    CPInit();								// Initialize the co-processor (sets I2C baud)
    
    // Run CP self test
    uint8_t selfTestData = 0x01;
    CPWrite(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);	// Send 1 to CP self test register to initiate
    CPRead(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);		// Read results (bits 7-4 hold the result)
    
    if (bit_is_clear(selfTestData, CP_SELFTEST_X509_BIT) ||     // Bit 7 = x.509 certificate: 0=not found, 1=found
    bit_is_clear(selfTestData, CP_SELFTEST_PK_BIT))				// Bit 6 = private key: 0=not found, 1=found
    {
        return FALSE;
    }
    
    // CP self test passed!
    
    // Get Authentication Version from co-processor
    CPRead(AUTH_PROTOCOL_MAJOR_VERSION, &mfiAuthStatus.protocolMajorVersion, 1);
    CPRead(AUTH_PROTOCOL_MINOR_VERSION, &mfiAuthStatus.protocolMinorVersion, 1);
    
    return TRUE;
}

BOOL MFIAuthenticate( void )
{
    uint8_t command;
    uint8_t commandDataLen;
    
    // Wait for request for certificate
    while (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_GetAccessoryAuthenticationInfo);
    
    // Get certificate data length from co-processor
    uint16_t certDataBytes;
    uint8_t lastSection;
    //uint8_t sectionIndex = 0;
    CPRead(AUTH_ACC_CERT_DATA_LENGTH, (uint8_t *)&certDataBytes, 2);
    certDataBytes = (certDataBytes<<8) + (certDataBytes>>8);             // certDataLength appears to come back flipped
    // Calculate the maximum number of sections that will be sent
    lastSection = certDataBytes/(uint8_t)MFI_AUTH_PAGE_SIZE;
    if (!(certDataBytes % (uint8_t)MFI_AUTH_PAGE_SIZE))
    {
        lastSection--;	// lastSection is 0-based, so catch exact page size case here
    }
    
    for (uint8_t i=0; i<=lastSection; i++)
    {
        // Packet data
        pCommandData[0] = mfiAuthStatus.protocolMajorVersion;
        pCommandData[1] = mfiAuthStatus.protocolMinorVersion;
        pCommandData[2] = i;
        pCommandData[3] = lastSection;
        
        // Get certificate data from the co-processor
        uint8_t bytesInSection = (certDataBytes > MFI_AUTH_PAGE_SIZE) ? MFI_AUTH_PAGE_SIZE : certDataBytes;
        CPRead(AUTH_ACC_CERT_DATA_START + i, pCommandData+4, bytesInSection);

        // Send certificate data to the iDevice
        mfiAuthStatus.transIDs.retAccessoryAuthenticationInfo = mfiStatus.transactionIDiPod;
//        MFISendCommand(IPOD_RetAccessoryAuthenticationInfo, pCommandData, bytesInSection+4, &mfiStatus.transactionIDiPod);
        
        // Wait for response and check if it's correct
        if (MFIReceiveCommand(&command, pCommandData, &commandDataLen))
        {
        
            if (i < lastSection)
            {
                if (command == IPOD_iPodAck && pCommandData[0] == IPOD_ACK_SUCCESS)
                {
                    certDataBytes -= bytesInSection;
                }
                else
                {
                    return FALSE;
                }
            }
            else if (command != IPOD_AckAccessoryAuthenticationInfo || pCommandData[0] != IPOD_ACK_SUCCESS)
            {
                return FALSE;
            }
        }
    }
    
    // Wait for request for signature
    while (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_GetAccessoryAuthenticationSignature);

    // Length should be 20, but they want 2 bytes for it on the CP
    uint16_t length = 20;
    
    uint16_t sigDataLen = 0;	// Should be 128 bytes, read from CP later
    
    // Send challenge data length to co-processor
    uint16_t lengthFlipped = (length>>8) + (length<<8);
    CPWrite(AUTH_CHALLENGE_DATA_LENGTH, (uint8_t *)&lengthFlipped, 2);
    // Send challenge data to co-processor
    CPWrite(AUTH_CHALLENGE_DATA, pCommandData, (uint8_t)length);
    
    // Start signature generation
    pCommandData[0] = 1;
    CPWrite(AUTH_CONTROL_STATUS, pCommandData, 1);
    
    // Check for result
    CPRead(AUTH_CONTROL_STATUS, pCommandData, 1);
    // Bit 7 is the error bit.  Status bits different from 1 also indicates failure
    if (BitIsSet(pCommandData[0], 7) || (((pCommandData[0] & CP_STATUS_BITMASK)>>4) != 1))
    {
        return FALSE;
    }
    
    // Read back signature
    CPRead(AUTH_SIGNATURE_DATA_LENGTH, (uint8_t *)&sigDataLen, 2);
    sigDataLen = (sigDataLen>>8) + (sigDataLen<<8);
    CPRead(AUTH_SIGNATURE_DATA, pCommandData, sigDataLen);
        
    // Send signature data to the iDevice
    mfiAuthStatus.transIDs.retAccessoryAuthenticationSignature = mfiStatus.transactionIDiPod;
    MFISendCommand(IPOD_RetAccessoryAuthenticationSignature, pCommandData, sigDataLen, &mfiStatus.transactionIDiPod);
    
    // Wait for request for signature
    if (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_AckAccessoryAuthenticationStatus || pCommandData[0] != IPOD_ACK_SUCCESS)
    {
        return FALSE;
    }
    
    return TRUE;
}

#endif //0

//////////////////////////////////////////////////////////////////////////
// Apple Authentication Co-processor Functions
//////////////////////////////////////////////////////////////////////////

void CPInit( void )
{
    /* *
    * The 2.0B co-processor runs at 40kHz, while the 2.0C can run at 400kHz.
    * This function will initialize to the lower speed, check the co-processor version
    * and reset the speed if necessary
    * */
    
    sbi(PORTC, PORTC6);											// Set CPRESET high to configure address
    TWIInit();													// Initialize first to low speed
    _delay_ms(10);									            // First data transmission must be >10ms after CPRESET goes high

    uint8_t deviceVersion = 0;
    CPRead(AUTH_DEVICE_VERSION, &deviceVersion, 1);				// Read Device Version from the CP
    if (deviceVersion == IPOD_CP_VERSION_2_0C)					// Set to high speed if it can handle it
    {
        TWI_SET_FREQUENCY(IPOD_CP_FREQUENCY_HIGH);
    }
    return;
}

BOOL CPWrite(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
       
    uint8_t msg[dataSize+2];
    msg[0] = IPOD_CP_INTERFACE & ~_BV(TWI_READ_BIT);			// First byte is the CP slave write address
    msg[1] = addr;												// Second byte is register address at which to begin writing
    if (dataSize > 0)
    {
        for (uint8_t i=0; i<dataSize; i++)
        {
            msg[i+2] = data[i];                      // Copy data from msg to data, skipping the address byte
        }
        
    }
    TWIStartTransceiverWithData(msg,dataSize+2);				// Send start and write address with data
    uint8_t retryCnt = 0;
    while (retryCnt < 5 && (twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK))
    {
        _delay_ms(100);						// Wait
        TWIStartTransceiver();				// Try again
        retryCnt++;
    }
    if (retryCnt == 5 && (twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK))
    {
        //Still failed.  Give up.
        return FALSE;
    }
    
    return TRUE;
}

BOOL CPRead(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
    CPWrite(addr, data, 0);										// First send write sequence with no data
    
    uint8_t msg[dataSize+1];									// Include one extra byte for the address
    msg[0] = IPOD_CP_INTERFACE | _BV(TWI_READ_BIT);				// Set the CP slave read address
    TWIStartTransceiverWithData(msg,dataSize+1);				// Send start and read address
    uint8_t retryCnt = 0;
    while (retryCnt < 5 && (twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK))
    {
        _delay_ms(100);						// Wait
        TWIStartTransceiver();				// Try again
        retryCnt++;
    }
    if (retryCnt == 5 && (twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK))
    {
        //Still failed.  Give up.
        return FALSE;
    }
    
    if(!TWIGetDataFromTransceiver(msg, dataSize+1))			// Get data from the buffer
    {
        return FALSE;										// Fail if unsuccessful
    }
    
    if (msg[0] == (IPOD_CP_INTERFACE | _BV(TWI_READ_BIT)))	// Check if the message is intended for us
    {
        // Bytes appear to need to be flipped
        for (uint8_t i=0; i<dataSize; i++)
        {
            data[i] = msg[i+1];                      // Copy data from msg to data, skipping the address byte
        }
    }
    else
    {
        data[0] = 0;
    }
    
    return TRUE;
}

