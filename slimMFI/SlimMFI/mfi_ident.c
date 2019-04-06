/*
* mfi_ident.c
*
* Created: 9/20/2012 4:57:57 PM
*  Author: Dan Kane
*/

#if 0

#include "mfi_ident.h"
#include "mfi.h"
#include "commands.h"

volatile struct mfiIdentStatus_t mfiIdentStatus;



BOOL MFIIdentify( void )
{
    //uint8_t data[IPOD_COMMAND_PAYLOAD_SIZE];
    uint8_t command = 0;
    uint8_t commandDataLen;
    
    // Send StartIDPS
    mfiIdentStatus.transIDs.startIDPS = mfiStatus.transactionIDRomo;
    MFISendCommand(IPOD_StartIDPS, pCommandData, 0, &mfiStatus.transactionIDRomo);
    //mfiStatus.transactionIDRomo++;
    
    // Wait for response and check if it's correct
    if (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_iPodAck || pCommandData[0] != IPOD_ACK_SUCCESS)
    {
        return FALSE;
    }
    
    // Send RequestTransportMaxPayloadSize
    mfiIdentStatus.transIDs.requestTransportMaxPayloadSize = mfiStatus.transactionIDRomo;
    MFISendCommand(IPOD_RequestTransportMaxPayloadSize, pCommandData, 0, &mfiStatus.transactionIDRomo);
    //mfiStatus.transactionIDRomo++;
    
    // Wait for response and check if it's correct
    if (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_ReturnTransportMaxPayloadSize)
    {
        return FALSE;
    }
    
    // Don't actually care what the max payload size is since we're below it

    
    // Send SetFIDTokenValues
    uint8_t tokensSent = 0;
    while(tokensSent < 11) {
        tokensSent += MFIIdentSendFIDTokens(tokensSent);
        // Wait for response and check if it's correct
        if (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_AckFIDTokenValues)
        {
            return FALSE;
        }
        // Check that all required tokens were accepted
        uint8_t start = 1;
        for (uint8_t i=0; i<pCommandData[0]; i++)
        {
            if (pCommandData[start+3] != 0x00)    // Check status of each token (byte 3)
            {
                return FALSE;
            }
            start += pCommandData[start] + 1;       // Add ack length + 1 to get to next ack
        }
    }
    
    // Final step is EndIDPS with status of 0x00 (success)
    pCommandData[0] = 0x00;
    mfiIdentStatus.transIDs.endIDPS = mfiStatus.transactionIDRomo;
    MFISendCommand(IPOD_EndIDPS, pCommandData, 1, &mfiStatus.transactionIDRomo);
    //mfiStatus.transactionIDRomo++;
    
    if (!MFIReceiveCommand(&command, pCommandData, &commandDataLen) || command != IPOD_IDPSStatus || pCommandData[0] != IPOD_ACK_SUCCESS)
    {
        return FALSE;
    }
    
    return TRUE;
}


uint8_t MFIIdentSendFIDTokens( uint8_t tokensSent )
{
    // FID Tokens
    uint8_t dataLen = 1;		// Start at 1 to skip numTokens byte (applied later)
    uint8_t numTokens = 0;
    uint8_t tokenStart;
    BOOL dataFull = FALSE;
    
    if (tokensSent == 0)
    {
        pCommandData[dataLen++] = 12;										// Size (13 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;								// Type
        pCommandData[dataLen++] = FID_IDENTIFY_TOKEN;						// Subtype
        pCommandData[dataLen++] = 1;										// Number of Lingoes (1)
        pCommandData[dataLen++] = IPOD_LINGO_GENERAL;						// General Lingo
        pCommandData[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE3;		// 4-byte Device Options (=0x02)
        pCommandData[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE2;
        pCommandData[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE1;
        pCommandData[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE0;
        CPRead(AUTH_DEVICE_ID, (pCommandData+dataLen), 4);                  // CoProcessor Device ID
        dataLen += 4;                                               // dataLen = 1 + 1 + 12 = 14
        numTokens++;
    }
    
    if (tokensSent <= 1)
    {
        pCommandData[dataLen++] = 10;										// Size (11 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;								// Type
        pCommandData[dataLen++] = FID_ACCESSORY_CAPS_TOKEN;					// Subtype
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE7;		// 8-byte Capabilities Mask (bits 63-56)
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE6;		// bits 55-48
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE5;		// bits 47-40
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE4;		// bits 39-32
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE3;		// bits 31-24
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE2;		// bits 23-16
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE1;		// bits 15-8, 9 is the only one we need (communication)
        pCommandData[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE0;		// bits 7-0
        numTokens++;
    }
    
    if ((tokensSent <= 2))
    {
        tokenStart = dataLen++;							// Store starting point to store size
        pCommandData[dataLen++] = FID_TYPE_0;					// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_NAME;			// Info Type
        char *tokenString = IPOD_ACCESSORY_NAME;
        uint8_t i = 0;
        while (tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE)
        {
            pCommandData[dataLen++] = tokenString[i++];	        // Copy tokenString into data
        }
        // Check if we overran the array
        if (dataLen >= IPOD_COMMAND_PAYLOAD_SIZE)
        {
            dataLen = tokenStart;			// This token is too big.  Remove it.
            dataFull = TRUE;				// Set flag
        }
        else
        {
            pCommandData[dataLen] = '\0';				            // Add back null terminator
            pCommandData[tokenStart] = dataLen - tokenStart;	    // Store size
            dataLen++;
            numTokens++;
        }
    }
    
    if ((tokensSent <= 3) && (dataLen+7) <= IPOD_COMMAND_PAYLOAD_SIZE)
    {
        pCommandData[dataLen++] = 6;									// Size (7 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;							// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;				// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_FIRMWARE_VERSION;		// Info Type
        pCommandData[dataLen++] = FW_VER_MAJOR;		// Firmware Version Major
        pCommandData[dataLen++] = FW_VER_MINOR;		// Firmware Version Minor
        pCommandData[dataLen++] = FW_VER_REVISION;		// Firmware Version Revision
        numTokens++;
    }
    
    if ((tokensSent <= 4) && (dataLen+7) <= IPOD_COMMAND_PAYLOAD_SIZE)
    {
        pCommandData[dataLen++] = 6;										// Size (7 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;								// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;					// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_HARDWARE_VERSION;			// Info Type
        pCommandData[dataLen++] = IPOD_ACCESSORY_HARDWARE_VERSION_MAJOR;	// Firmware Version Major
        pCommandData[dataLen++] = IPOD_ACCESSORY_HARDWARE_VERSION_MINOR;	// Firmware Version Minor
        pCommandData[dataLen++] = IPOD_ACCESSORY_HARDWARE_VERSION_REV;		// Firmware Version Revision
        numTokens++;
    }
    
    if ((tokensSent <= 5) && !dataFull)
    {
        tokenStart = dataLen++;							// Store starting point to store size
        pCommandData[dataLen++] = FID_TYPE_0;					// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_MANUFACTURER;	// Info Type
        char *tokenString = IPOD_ACCESSORY_MANUFACTURER;
        uint8_t i = 0;
        while (tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE)
        {
            pCommandData[dataLen++] = tokenString[i++];	        // Copy tokenString into data
        }
        // Check if we overran the array
        if (dataLen >= IPOD_COMMAND_PAYLOAD_SIZE)
        {
            dataLen = tokenStart;			// This token is too big.  Remove it.
            dataFull = TRUE;				// Set flag
        }
        else
        {
            pCommandData[dataLen] = '\0';				            // Add back null terminator
            pCommandData[tokenStart] = dataLen - tokenStart;	    // Store size
            dataLen++;
            numTokens++;
        }
    }
    
    if ((tokensSent <= 6) && !dataFull)
    {
        tokenStart = dataLen++;							// Store starting point to store size
        pCommandData[dataLen++] = FID_TYPE_0;					// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_MODEL_NUMBER;	// Info Type
        char *tokenString = IPOD_ACCESSORY_MODEL_NUMBER;
        uint8_t i = 0;
        while (tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE)
        {
            pCommandData[dataLen++] = tokenString[i++];	        // Copy tokenString into data
        }
        // Check if we overran the array
        if (dataLen >= IPOD_COMMAND_PAYLOAD_SIZE)
        {
            dataLen = tokenStart;			// This token is too big.  Remove it.
            dataFull = TRUE;				// Set flag
        }
        else
        {
            pCommandData[dataLen] = '\0';				            // Add back null terminator
            pCommandData[tokenStart] = dataLen - tokenStart;	    // Store size
            dataLen++;
            numTokens++;
        }
    }
    
    if ((tokensSent <= 7) && (dataLen+6) <= IPOD_COMMAND_PAYLOAD_SIZE)
    {
        pCommandData[dataLen++] = 5;								// Size (6 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;						// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;			// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_MAX_PAYLOAD_SIZE;	// Info Type
        pCommandData[dataLen++] = 0x00;								// High byte (0)
        pCommandData[dataLen++] = IPOD_COMMAND_PAYLOAD_SIZE;		// Low byte (136)
        numTokens++;
    }
    
    if ((tokensSent <= 8) && (dataLen+8) <= IPOD_COMMAND_PAYLOAD_SIZE)
    {
        pCommandData[dataLen++] = 7;										// Size (8 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;								// Type
        pCommandData[dataLen++] = FID_ACCESSORY_INFO_TOKEN;					// Subtype
        pCommandData[dataLen++] = FID_ACC_INFO_RF_CERTIFICATIONS;			// Info Type
        pCommandData[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE3;	// 32-bit mask (MSB)
        pCommandData[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE2;
        pCommandData[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE1;
        pCommandData[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE0;	// LSB
        numTokens++;
    }
    
    if ((tokensSent <= 9) && !dataFull)
    {
        tokenStart = dataLen++;							// Store starting point to store size
        pCommandData[dataLen++] = FID_TYPE_0;					// Type
        pCommandData[dataLen++] = FID_EA_PROTOCOL_TOKEN;		// Subtype
        pCommandData[dataLen++] = IPOD_ACCESSORY_EA_INDEX;
        char *tokenString = IPOD_ACCESSORY_EA_STRING;
        uint8_t i = 0;
        while (tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE)
        {
            pCommandData[dataLen++] = tokenString[i++];	        // Copy tokenString into data
        }
        // Check if we overran the array
        if (dataLen >= IPOD_COMMAND_PAYLOAD_SIZE)
        {
            dataLen = tokenStart;			// This token is too big.  Remove it.
            dataFull = TRUE;				// Set flag
        }
        else
        {
            pCommandData[dataLen] = '\0';				            // Add back null terminator
            pCommandData[tokenStart] = dataLen - tokenStart;	    // Store size
            dataLen++;
            numTokens++;
        }
    }
    
    if ((tokensSent <= 10) && (dataLen+14) <= IPOD_COMMAND_PAYLOAD_SIZE)
    {
        pCommandData[dataLen++] = 13;								// Size (14 bytes in field)
        pCommandData[dataLen++] = FID_TYPE_0;						// Type
        pCommandData[dataLen++] = FID_BUNDLE_SEED_ID_PREF_TOKEN;	// Subtype
        char *tokenString = IPOD_ACCESSORY_BUNDLE_SEED_ID;	// 11 bytes incl. '\0'
        for (uint8_t i=0; i<11; i++)
        {
            pCommandData[dataLen++] = tokenString[i];	// Copy tokenString into data
        }
        numTokens++;
    }
    
    if (numTokens > 0)
    {
        pCommandData[0] = numTokens;
        
        mfiIdentStatus.transIDs.setFIDTokenValues = mfiStatus.transactionIDRomo;
        MFISendCommand(IPOD_SetFIDTokenValues, pCommandData, dataLen, &mfiStatus.transactionIDRomo);
    }
    
    return numTokens;
}

/*
void MFIIdentSendFIDTokens( void )
{
    // FID Tokens
    uint8_t strLen = 0;
    pCommandData[0] = 4;        // Number of tokens
    
    // Identify Token
    pCommandData[1] = 12;										// Size (13 bytes in field)
    pCommandData[2] = FID_TYPE_0;								// Type
    pCommandData[3] = FID_IDENTIFY_TOKEN;						// Subtype
    pCommandData[4] = 1;										// Number of Lingoes (1)
    pCommandData[5] = IPOD_LINGO_GENERAL;						// General Lingo
    pCommandData[6] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE3;		// 4-byte Device Options (=0x02)
    pCommandData[7] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE2;
    pCommandData[8] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE1;
    pCommandData[9] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE0;
    CPRead(AUTH_DEVICE_ID, (pCommandData+10), 4);                  // CoProcessor Device ID
    
    // Accessory Capabilities Token
    pCommandData[14] = 10;										// Size (11 bytes in field)
    pCommandData[15] = FID_TYPE_0;								// Type
    pCommandData[16] = FID_ACCESSORY_CAPS_TOKEN;					// Subtype
    pCommandData[17] = IPOD_ACCESSORY_CAPABILITIES_BYTE7;		// 8-byte Capabilities Mask (bits 63-56)
    pCommandData[18] = IPOD_ACCESSORY_CAPABILITIES_BYTE6;		// bits 55-48
    pCommandData[19] = IPOD_ACCESSORY_CAPABILITIES_BYTE5;		// bits 47-40
    pCommandData[20] = IPOD_ACCESSORY_CAPABILITIES_BYTE4;		// bits 39-32
    pCommandData[21] = IPOD_ACCESSORY_CAPABILITIES_BYTE3;		// bits 31-24
    pCommandData[22] = IPOD_ACCESSORY_CAPABILITIES_BYTE2;		// bits 23-16
    pCommandData[23] = IPOD_ACCESSORY_CAPABILITIES_BYTE1;		// bits 15-8, 9 is the only one we need (communication)
    pCommandData[24] = IPOD_ACCESSORY_CAPABILITIES_BYTE0;		// bits 7-0
    
    // Bundle Seed ID Token
    pCommandData[25] = 13;								// Size (14 bytes in field)
    pCommandData[26] = FID_TYPE_0;						// Type
    pCommandData[27] = FID_BUNDLE_SEED_ID_PREF_TOKEN;	// Subtype
    char *tokenString = IPOD_ACCESSORY_BUNDLE_SEED_ID;	// 11 bytes incl. '\0'
    for (uint8_t i=0; i<11; i++)
    {
        pCommandData[28+i] = tokenString[i];	// Copy tokenString into data
    }
    
    // EA String Token
    pCommandData[40] = FID_TYPE_0;					// Type
    pCommandData[41] = FID_EA_PROTOCOL_TOKEN;		// Subtype
    pCommandData[42] = IPOD_ACCESSORY_EA_INDEX;
    char *eaString = IPOD_ACCESSORY_EA_STRING;
    while (eaString[strLen] != '\0')
    {
        pCommandData[43+strLen] = eaString[strLen];	        // Copy tokenString into data
        strLen++;
    }
    pCommandData[43+strLen++] = '\0';				        // Add back null terminator
    pCommandData[39] = strLen+3;	                            // Store size

    // Send tokens    
    mfiIdentStatus.transIDs.setFIDTokenValues = mfiStatus.transactionIDRomo;
    MFISendCommand(IPOD_SetFIDTokenValues, pCommandData, strLen+43, &mfiStatus.transactionIDRomo);
    
}
*/

#endif //0