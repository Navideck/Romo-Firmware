/*
* mfi_ident.c
*
* Created: 9/20/2012 4:57:57 PM
*  Author: Dan Kane
*/

#include "main.h"
#include "mfi_ident.h"
#include "mfi.h"
#include "timers.h"

volatile mfiIdentStatus_t mfiIdentStatus;


void MFIIdentInit()
{
    mfiIdentStatus.acked.all = 0;
    mfiIdentStatus.fidTokensAcked.all = 0;
    mfiIdentStatus.fidInfoTokensSent.all = 0;
}

void MFIIdentTasks()
{
    uint8_t data[IPOD_COMMAND_PAYLOAD_SIZE];
    
    if(mfiStatus.flags.needsIDPS && !mfiStatus.flags.waitingForAck) {
        // Reset everything in case of RequestIdentify
        mfiStatus.communication.isIdentifying = TRUE;
        mfiStatus.communication.isAuthenticating = FALSE;
        mfiIdentStatus.acked.all = 0;
        mfiIdentStatus.fidTokensAcked.all = 0;
        mfiIdentStatus.fidInfoTokensSent.all = 0;
        
        // Start IDPS
        mfiIdentStatus.transIDs.startIDPS = mfiStatus.transactionIDRomo;
        MFISendCommand(IPOD_StartIDPS, data, 0, mfiStatus.transactionIDRomo++);
        
        ackWaitCnt = 0;
        mfiStatus.ackWaitTicks = MS_TO_TICKS(500);
        mfiStatus.flags.waitingForAck = TRUE;
        
        mfiStatus.flags.needsIDPS = FALSE;
        return;
    }
    
    if(mfiStatus.communication.isIdentifying && !mfiStatus.flags.waitingForAck) {
        // Check ack'd commands in reverse order to find next in sequence
        
        if(mfiIdentStatus.acked.endIDPS) {
            mfiStatus.communication.isIdentifying = FALSE;
            //mfiStatus.flags.needsAuthentication = TRUE;
            mfiStatus.flags.waitingForAck = TRUE;
            
        }
        else if(mfiIdentStatus.acked.setFIDTokenValues) {
            // Final step is EndIDPS with status of 0x00 (success)
            data[0] = 0x00;
            mfiIdentStatus.transIDs.endIDPS = mfiStatus.transactionIDRomo;
            MFISendCommand(IPOD_EndIDPS, data, 1, mfiStatus.transactionIDRomo++);
            mfiStatus.flags.waitingForAck = TRUE;
            
        }
        else if(mfiIdentStatus.acked.getiPodOptionsForLingo) {
            // Next step is SetFIDTokenValues
            // TESTING (skip this step)
            //mfiIdentStatus.acked.setFIDTokenValues = TRUE;
            
            MFIIdentSendFIDTokens(data);
            mfiStatus.flags.waitingForAck = TRUE;
            
        }
        else if(mfiIdentStatus.acked.requestTransportMaxPayloadSize) {
            // Next step is GetiPodOptionsForLingo
            data[0] = IPOD_LINGO_GENERAL;
            mfiIdentStatus.transIDs.getiPodOptionsForLingo = mfiStatus.transactionIDRomo;
            MFISendCommand(IPOD_GetiPodOptionsForLingo, data, 1, mfiStatus.transactionIDRomo++);
            mfiStatus.flags.waitingForAck = TRUE;

        }
        else if(mfiIdentStatus.acked.startIDPS) {
            // Next step is RequestTransportMaxPayloadSize
            mfiIdentStatus.transIDs.requestTransportMaxPayloadSize = mfiStatus.transactionIDRomo;
            MFISendCommand(IPOD_RequestTransportMaxPayloadSize, data, 0, mfiStatus.transactionIDRomo++);
        }
        ackWaitCnt = 0;
        mfiStatus.ackWaitTicks = MS_TO_TICKS(500);
        mfiStatus.flags.waitingForAck = TRUE;
    }
}


void MFIIdentSendFIDTokens(uint8_t *data)
{
    // FID Tokens
    //uint8_t data[IPOD_COMMAND_PAYLOAD_SIZE];
    uint8_t dataLen = 1;		// Start at 1 to skip numTokens byte (applied later)
    uint8_t numTokens = 0;
    uint8_t tokenStart;
    BOOL dataFull = FALSE;
    
    if(!mfiIdentStatus.fidTokensAcked.identifyToken) {
        data[dataLen++] = 12;										// Size (13 bytes in field)
        data[dataLen++] = FID_TYPE_0;								// Type
        data[dataLen++] = FID_IDENTIFY_TOKEN;						// Subtype
        data[dataLen++] = 1;										// Number of Lingoes (1)
        data[dataLen++] = IPOD_LINGO_GENERAL;						// General Lingo
        data[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE3;		// 4-byte Device Options (=0x02)
        data[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE2;
        data[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE1;
        data[dataLen++] = IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE0;
        CPRead(AUTH_DEVICE_ID, (data+dataLen), 4);                  // CoProcessor Device ID
        dataLen += 4;                                               // dataLen = 1 + 1 + 12 = 14
        numTokens++;
    }
    
    if(!mfiIdentStatus.fidTokensAcked.accessoryCapsToken) {
        data[dataLen++] = 10;										// Size (11 bytes in field)
        data[dataLen++] = FID_TYPE_0;								// Type
        data[dataLen++] = FID_ACCESSORY_CAPS_TOKEN;					// Subtype
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE7;		// 8-byte Capabilities Mask (bits 63-56)
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE6;		// bits 55-48
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE5;		// bits 47-40
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE4;		// bits 39-32
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE3;		// bits 31-24
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE2;		// bits 23-16
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE1;		// bits 15-8, 9 is the only one we need (communication)
        data[dataLen++] = IPOD_ACCESSORY_CAPABILITIES_BYTE0;		// bits 7-0
        numTokens++;
    }
    
    if(!mfiIdentStatus.fidTokensAcked.accessoryInfoTokens) {
        if(!mfiIdentStatus.fidInfoTokensSent.name && !dataFull) {
            tokenStart = dataLen++;							// Store starting point to store size
            //data[dataLen++] = ??;							// Size (added later)
            data[dataLen++] = FID_TYPE_0;					// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		// Subtype
            data[dataLen++] = FID_ACC_INFO_NAME;			// Info Type
            char *tokenString = IPOD_ACCESSORY_NAME;
            uint8_t i = 0;
            while(tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE) {
                data[dataLen++] = tokenString[i++];	        // Copy tokenString into data
            }
            // Check if we overran the array
            if(dataLen >= IPOD_COMMAND_PAYLOAD_SIZE) {
                dataLen = tokenStart;			// This token is too big.  Remove it.
                dataFull = TRUE;				// Set flag
            }
            else {
                data[dataLen] = '\0';				            // Add back null terminator
                data[tokenStart] = dataLen - tokenStart;	    // Store size
                dataLen++;
                numTokens++;
            }
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.firmwareVersion && (dataLen+7) <= IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = 6;									// Size (7 bytes in field)
            data[dataLen++] = FID_TYPE_0;							// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;				// Subtype
            data[dataLen++] = FID_ACC_INFO_FIRMWARE_VERSION;		// Info Type
            data[dataLen++] = FIRMWARE_VERSION_MAJOR;               // Firmware Version Major
            data[dataLen++] = FIRMWARE_VERSION_MINOR;               // Firmware Version Minor
            data[dataLen++] = FIRMWARE_VERSION_REVISION;            // Firmware Version Revision
            numTokens++;
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.hardwareVersion && (dataLen+7) <= IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = 6;									// Size (7 bytes in field)
            data[dataLen++] = FID_TYPE_0;							// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;				// Subtype
            data[dataLen++] = FID_ACC_INFO_HARDWARE_VERSION;		// Info Type
            data[dataLen++] = devInfo.hardwareVersion.major;	    // Hardware Version Major
            data[dataLen++] = devInfo.hardwareVersion.minor;	    // Hardware Version Minor
            data[dataLen++] = devInfo.hardwareVersion.revision;		// Hardware Version Revision
            numTokens++;
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.manufacturer && !dataFull) {
            tokenStart = dataLen++;						  	        // Store starting point to store size
            //data[dataLen++] = ??;							        // Size (added later)
            data[dataLen++] = FID_TYPE_0;					        // Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		        // Subtype
            data[dataLen++] = FID_ACC_INFO_MANUFACTURER;	        // Info Type
            char *tokenString = IPOD_ACCESSORY_MANUFACTURER;
            uint8_t i = 0;
            while(tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE) {
                data[dataLen++] = tokenString[i++];	        // Copy tokenString into data
            }
            // Check if we overran the array
            if(dataLen >= IPOD_COMMAND_PAYLOAD_SIZE) {
                dataLen = tokenStart;			// This token is too big.  Remove it.
                dataFull = TRUE;				// Set flag
            }
            else {
                data[dataLen] = '\0';				            // Add back null terminator
                data[tokenStart] = dataLen - tokenStart;	    // Store size
                dataLen++;
                numTokens++;
            }
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.modelNumber && !dataFull) {
            char *tokenString;
            tokenStart = dataLen++;							// Store starting point to store size
            //data[dataLen++] = ??;							// Size (added later)
            data[dataLen++] = FID_TYPE_0;					// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;		// Subtype
            data[dataLen++] = FID_ACC_INFO_MODEL_NUMBER;	// Info Type
            if(devInfo.type == RMDeviceTypeApple30Pin) {
                tokenString = "3b";
            }
            else if(devInfo.type == RMDeviceTypeAppleLightning) {
                tokenString = "3l";
            }
            else {
                tokenString = IPOD_ACCESSORY_MODEL_NUMBER;
            }
            uint8_t i = 0;
            while(tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE) {
                data[dataLen++] = tokenString[i++];	        // Copy tokenString into data
            }
            // Check if we overran the array
            if(dataLen >= IPOD_COMMAND_PAYLOAD_SIZE) {
                dataLen = tokenStart;			// This token is too big.  Remove it.
                dataFull = TRUE;				// Set flag
            }
            else {
                data[dataLen] = '\0';				            // Add back null terminator
                data[tokenStart] = dataLen - tokenStart;	    // Store size
                dataLen++;
                numTokens++;
            }
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.serialNumber && (dataLen+24) <= IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = 23;                               // Size (24 bytes in field)
            data[dataLen++] = FID_TYPE_0;                       // Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;         // Subtype
            data[dataLen++] = FID_ACC_INFO_SERIAL_NUMBER;       // Info Type
            for(uint8_t i=0; i<19; i++) {
                data[dataLen++] = devInfo.serialNumber[i];      // Serial Number
            }
            data[dataLen++] = '\0';
            numTokens++;
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.maxPayloadSize && (dataLen+6) <= IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = 5;								// Size (6 bytes in field)
            data[dataLen++] = FID_TYPE_0;						// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;			// Subtype
            data[dataLen++] = FID_ACC_INFO_MAX_PAYLOAD_SIZE;	// Info Type
            data[dataLen++] = 0x00;								// High byte (0)
            data[dataLen++] = IPOD_COMMAND_PAYLOAD_SIZE;		// Low byte (136)
            numTokens++;
        }
        
        // Maybe not needed?
        if(!mfiIdentStatus.fidInfoTokensSent.accStatus) {// && (dataLen+8) <= IPOD_COMMAND_PAYLOAD_SIZE
            mfiIdentStatus.fidInfoTokensSent.accStatus = TRUE;
        }
        
        if(!mfiIdentStatus.fidInfoTokensSent.rfCertifications && (dataLen+8) <= IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = 7;										// Size (8 bytes in field)
            data[dataLen++] = FID_TYPE_0;								// Type
            data[dataLen++] = FID_ACCESSORY_INFO_TOKEN;					// Subtype
            data[dataLen++] = FID_ACC_INFO_RF_CERTIFICATIONS;			// Info Type
            data[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE3;	// 32-bit mask (MSB)
            data[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE2;
            data[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE1;
            data[dataLen++] = IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE0;	// LSB
            numTokens++;
        }
    }	// End if (!mfiIdentStatus.fidTokensAcked.accessoryInfoTokens)


    if(!mfiIdentStatus.fidTokensAcked.eaProtocolToken && !dataFull) {
        //mfiIdentStatus.fidTokensAcked.eaProtocolToken = TRUE;
        tokenStart = dataLen++;							// Store starting point to store size
        //data[dataLen++] = ??;							// Size (added later)
        data[dataLen++] = FID_TYPE_0;					// Type
        data[dataLen++] = FID_EA_PROTOCOL_TOKEN;		// Subtype
        data[dataLen++] = IPOD_ACCESSORY_EA_INDEX;
        char *tokenString = IPOD_ACCESSORY_EA_STRING;
        uint8_t i = 0;
        while(tokenString[i] != '\0' && dataLen < IPOD_COMMAND_PAYLOAD_SIZE) {
            data[dataLen++] = tokenString[i++];	        // Copy tokenString into data
        }
        // Check if we overran the array
        if(dataLen >= IPOD_COMMAND_PAYLOAD_SIZE) {
            dataLen = tokenStart;			// This token is too big.  Remove it.
            dataFull = TRUE;				// Set flag
        }
        else {
            data[dataLen] = '\0';				            // Add back null terminator
            data[tokenStart] = dataLen - tokenStart;	    // Store size
            dataLen++;
            numTokens++;
        }
    }
    
    if(!mfiIdentStatus.fidTokensAcked.bundleSeedIDPrefToken && (dataLen+14) <= IPOD_COMMAND_PAYLOAD_SIZE) {
        data[dataLen++] = 13;								// Size (14 bytes in field)
        data[dataLen++] = FID_TYPE_0;						// Type
        data[dataLen++] = FID_BUNDLE_SEED_ID_PREF_TOKEN;	// Subtype
        char *tokenString = IPOD_ACCESSORY_BUNDLE_SEED_ID;	// 11 bytes incl. '\0'
        for(uint8_t i=0; i<11; i++) {
            data[dataLen++] = tokenString[i];	// Copy tokenString into data
        }
        numTokens++;
    }
    
    if(!mfiIdentStatus.fidTokensAcked.eaProtocolMetadataToken && (dataLen+5) <= IPOD_COMMAND_PAYLOAD_SIZE) {
        data[dataLen++] = 4;								// Size (5 bytes in field)
        data[dataLen++] = FID_TYPE_0;						// Type
        data[dataLen++] = FID_EA_PROTOCOL_METADATA_TOKEN;	// Subtype
        data[dataLen++] = 0x01;								// Protocol Index
        data[dataLen++] = 0x01;								// 0x01 = try to find matching app
        numTokens++;
    }
    
    if(numTokens > 0) {
        data[0] = numTokens;
        
        mfiIdentStatus.transIDs.setFIDTokenValues = mfiStatus.transactionIDRomo;
        MFISendCommand(IPOD_SetFIDTokenValues, data, dataLen, mfiStatus.transactionIDRomo++);
    }
}