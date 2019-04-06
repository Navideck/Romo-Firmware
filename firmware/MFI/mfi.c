/*
* mfi.c
*
* Created: 9/19/2012 11:02:51 AM
*  Author: Dan Kane
*/

#include "main.h"
#include "mfi.h"
#include "commands.h"
#include "ringbuffer.h"
#include "util.h"
#include "timers.h"
#include "leds.h"

volatile mfiStatus_t mfiStatus;
volatile mfiPacket_t mfiPendingPacket;
uint8_t mfiCommand = 0xFF;        // Initialize to non-existant command
uint8_t mfiCommandDataLen;
uint8_t mfiAckPayloadSize;
uint8_t mfiAckBuf[CMD_MAX_PAYLOAD + 4];
uint8_t mfiResendCount;
MFIReadState mfiReadState = MFI_READ_STATE_INIT;

BOOL MFIInit(void)
{
    mfiStatus.flags.all = 0;
    mfiStatus.communication.all = 0;
    mfiStatus.ackWaitTicks = MS_TO_TICKS(1000);
    
    MFIIdentInit();

    mfiStatus.sessionIDiPod = bootloaderSessionID;
    mfiStatus.transactionIDRomo = bootloaderTransactionID;

    #if defined(MFI_REAUTH_IN_MAIN) || defined(DEBUG)

    // Initialize authentication co-processor
    if(!MFIAuthInit()) {
        // PROBLEM WITH AUTH!!
        ledControl.mode = RMLedModeBlink;
        LEDSetBlink(100,100);
        while(1);
    }

    // If we have a valid sessionID, then we probably authed in the bootloader
    if(mfiStatus.transactionIDRomo == 0) {
        // UART startup
        delay_ms(80);					// Wait 80ms
        MutexLockOutBuf();
        PutByteOutBuf(SYNC_BYTE);		// Send sync byte
        MutexUnlockOutBuf();

        delay_ms(20);					// Wait 20ms
        
        
        // Start IDPS
        // If we are coming from
        //mfiStatus.flags.needsIDPS = mfiStatus.transactionIDRomo ? FALSE : TRUE;
        mfiStatus.flags.needsIDPS = TRUE;
    }
    #else
    mfiStatus.flags.needsIDPS = FALSE;
    #endif
    
    return TRUE;
}


void MFIHandleAck(void)
{

    // iPodAck has 2 bytes of data
    if(mfiCommandDataLen >= 2) {
        // Command status
        switch(pCommandData[0]) {
            #if 1
            case IPOD_ACK_SUCCESS:
            // Command being ack'd
            switch(pCommandData[1]) {
                #if 1 //code folding
                case IPOD_StartIDPS:
                // Verify transaction ID of the incoming response
                if(mfiIdentStatus.transIDs.startIDPS == mfiStatus.transactionIDiPod) {
                    mfiIdentStatus.acked.startIDPS = TRUE;
                    mfiStatus.flags.needsIDPS = FALSE;
                    mfiStatus.flags.waitingForAck = FALSE;
                }
                break;

                case IPOD_RetAccessoryAuthenticationInfo:
                // Prepare for next chunk of certificate data
                // Make sure this is the right iPodAck condition during debugging
                if(mfiAuthStatus.transIDs.retAccessoryAuthenticationInfo == mfiStatus.transactionIDiPod) {
                    mfiAuthStatus.sectionIndex++;
                    // Clear ack so that we send next section
                    mfiAuthStatus.acked.retAccessoryAuthenticationInfo = FALSE;
                    mfiStatus.flags.waitingForAck = FALSE;
                }
                break;

                case IPOD_AccessoryDataTransfer:
                if(mfiPendingPacket.transactionID == mfiStatus.transactionIDiPod) {
                    mfiStatus.flags.waitingForAck = FALSE;
                }
                break;
                
                default:
                mfiStatus.flags.waitingForAck = FALSE;
                break;

                #endif
            }
            break;

            case IPOD_ACK_UNKNOWN_DATABASE_CATEGORY:
            case IPOD_ACK_COMMAND_FAILED:
            if(pCommandData[1] == IPOD_SetFIDTokenValues) {
                // Failed setting FID token values
                // Restart IDPS?
                mfiStatus.flags.needsIDPS = TRUE;
            }
            break;

            case IPOD_ACK_OUT_OF_RESOURCES:
            break;

            case IPOD_ACK_BAD_PARAMETER:
            // switch on command being ack'd
            switch(pCommandData[1]) {
                #if 1 //code folding
                case IPOD_StartIDPS:
                // Does not support IDPS
                // Send EndIDPS with status 0x02 (abandoning IDPS)
                mfiAckBuf[0] = 0x02;
                MFISendCommand(IPOD_EndIDPS, mfiAckBuf, 1, mfiStatus.transactionIDRomo++);
                // NEED TO SEND IDENTIFY DEVICE LINGOES WITH ALL PARAMETERS SET TO 0XFF (IDPS REQUIRED)
                // OR NEED TO SEND IDENTIFY DEVICE LINGOES WITH LINGO MASK SET FOR GENERAL ONLY, NO OPTIONS, AND DEVICEID OF 0X00 (IDPS NOT REQUIRED)
                mfiStatus.flags.waitingForAck = FALSE;
                break;

                case IPOD_RequestTransportMaxPayloadSize:
                // Use default (506 bytes). We're using 136.  Record ack.
                mfiIdentStatus.acked.requestTransportMaxPayloadSize = TRUE;
                mfiStatus.flags.waitingForAck = FALSE;
                break;

                case IPOD_RequestApplicationLaunch:
                // Retry?
                break;

                default:
                break;

                #endif
            }
            break;

            case IPOD_ACK_UNKNOWN_ID:
            break;

            case IPOD_ACK_COMMAND_PENDING:
            // Need to wait the specified timeout and then retry
            // 32-bit wait time after
            if(mfiCommandDataLen >= 6) {
                //ackWaitCnt = 0;
                //mfiStatus.ackWaitTicks = MS_TO_TICKS(*((uint32_t *)(pCommandData+2)));
                // STILL NEED TO FIGURE THIS OUT
            }
            break;

            case IPOD_ACK_NOT_AUTHENTICATED:
            case IPOD_ACK_BAD_AUTHENTICATION_VERSION:
            case IPOD_ACK_ACCESSORY_POWER_MODE_REQUEST_FAILED:
            case IPOD_ACK_CERTIFICATE_INVALID:
            case IPOD_ACK_CERTIFICATE_PERMISSIONS_INVALID:
            case IPOD_ACK_FILE_IS_IN_USE:
            case IPOD_ACK_INVALID_FILE_HANDLE:
            case IPOD_ACK_DIRECTORY_NOT_EMPTY:
            break;

            case IPOD_ACK_OPERATION_TIMED_OUT:
            // Did not complete IDPS in time
            mfiStatus.flags.fatalError = TRUE;
            break;

            case IPOD_ACK_COMMAND_UNAVAILABLE:
            case IPOD_ACK_INVALID_ACCESSORY_RESISTOR:
            case IPOD_ACK_ACCESSORY_NOT_GROUNDED:
            case IPOD_ACK_SUCCESS_MULTISECTION_DATA:
            break;

            case IPOD_ACK_MAXIMUM_ACCESSORY_CONNECTIONS:
            // Maximum accessories for device. Must abort.
            // Send EndIDPS with status 0x02 (abandoning IDPS)
            mfiAckBuf[0] = 0x02;
            MFISendCommand(IPOD_EndIDPS, mfiAckBuf, 1, mfiStatus.transactionIDRomo++);
            break;

            case IPOD_ACK_SessionWriteFailure:
            case IPOD_ACK_IPOD_OUT_MODE_ENTRY_ERROR:
            default:
            break;

            #endif
        }
    }
}


void MFIHandleAckAccessoryAuthenticationInfo(void)
{
    //mfiStatus.flags.waitingForAck = FALSE;
    
    // Received after entire x.509 cert has been sent and processed by iDevice
    if((mfiAuthStatus.transIDs.retAccessoryAuthenticationInfo == mfiStatus.transactionIDiPod) &&
    (mfiCommandDataLen >= 1) ) {
        switch(pCommandData[0]) {
            case AUTH_STATUS_SUCCESS:
            mfiAuthStatus.acked.retAccessoryAuthenticationInfo = TRUE;
            break;

            case AUTH_STATUS_CERT_TOO_LONG:
            case AUTH_STATUS_UNSUPPORTED:
            case AUTH_STATUS_CERT_INVALID:
            case AUTH_STATUS_BAD_PERMISSIONS:
            // Enter failure state -- maybe figure out better responses to these conditions later
            mfiStatus.flags.fatalError = TRUE;
            break;

            default:
            break;
        }
    }
}


void MFIHandleAckAccessoryAuthenticationStatus(void)
{
    mfiStatus.flags.waitingForAck = FALSE;
    
    if((mfiAuthStatus.transIDs.retAccessoryAuthenticationSignature == mfiStatus.transactionIDiPod) &&
    (mfiCommandDataLen >= 1) ) {
        // Success!
        if(pCommandData[0] == 0x00) {
            mfiStatus.communication.isAuthenticating = FALSE;
            mfiStatus.flags.authComplete = TRUE;
            mfiAuthStatus.acked.retAccessoryAuthenticationSignature = TRUE;
        }
        else {
            // Failed!
            mfiStatus.flags.fatalError = TRUE;
        }
        mfiAuthStatus.acked.retAccessoryAuthenticationSignature = TRUE;
    }
}


void MFIHandleAckFIDTokens(void)
{    
    // Check which tokens were acked and the ack status
    if(mfiIdentStatus.transIDs.setFIDTokenValues == mfiStatus.transactionIDiPod) {
        uint8_t numTokenAcks = pCommandData[0];
        uint8_t tokenStart = 1;
        for(uint8_t i=0; i<numTokenAcks; i++) {
            uint8_t length = pCommandData[tokenStart];
            uint8_t fidType = pCommandData[tokenStart+1];
            uint8_t fidSubtype = pCommandData[tokenStart+2];
            uint8_t ackStatus = pCommandData[tokenStart+3];
            if(fidSubtype == FID_ACCESSORY_INFO_TOKEN && (ackStatus == 0 || ackStatus == 3)) {
                uint8_t infoType = pCommandData[tokenStart+4];
                // Bit positions in struct match info token value
                mfiIdentStatus.fidInfoTokensSent.all |= _BV(infoType);
                // Check if all info tokens have acked sent
                if((mfiIdentStatus.fidInfoTokensSent.all & FID_REQ_ACC_INFO_TOKEN_MASK) == FID_REQ_ACC_INFO_TOKEN_MASK) {
                    mfiIdentStatus.fidTokensAcked.accessoryInfoTokens = TRUE;
                }
            }
            else if(fidType == FID_TYPE_0 && (ackStatus == 0 || ackStatus == 3)) {
                // 0 = success.  3 = unsupported -- mark acked so we stop trying
                // Bit positions in struct match token subtype
                mfiIdentStatus.fidTokensAcked.all |= _BV(fidSubtype);
            }
            // CHECK FOR ACKSTATUS=1 (FAILED)?  AND DO WHAT?
            tokenStart += length+1;        // Prepare for next token
        }
        if((mfiIdentStatus.fidTokensAcked.all & FID_REQUIRED_TOKEN_MASK) == FID_REQUIRED_TOKEN_MASK) {
            // All tokens complete, record overall ack
            mfiIdentStatus.acked.setFIDTokenValues = TRUE;
        }
        mfiStatus.flags.waitingForAck = FALSE;
    }
}


void MFIHandleIDPSStatus(void)
{
    // Response to EndIDPS
    if(mfiIdentStatus.transIDs.endIDPS == mfiStatus.transactionIDiPod) {
        // IDPS status
        switch(pCommandData[0]) {
            case 0:        // Success
            mfiIdentStatus.acked.endIDPS = TRUE;
            break;

            case 1: // FIDTokenValue failure, IDPS fails
            case 2: // FIDTokenValue missing, IDPS fails
            case 3: // FIDTokenValue rejected, IDPS fails
            case 4: // Time limit not exceeded
            case 7: // Error in FID Tokens, IDPS fails
            mfiStatus.flags.needsIDPS = TRUE; // Retry IDPS until we time out
            break;

            case 5: // Time limit exceeded, cannot retry
            case 6: // Device triggered IDPS failure
            // IDPS will not succeed.  Enter failure state.
            mfiStatus.communication.isIdentifying = FALSE;
            mfiStatus.communication.isAuthenticating = FALSE;
            mfiStatus.flags.fatalError = TRUE;
            break;

            default:
            break;
        }
    }
}


void MFIHandleOpenDataSessionForProtocol(void)
{
    // App initiated data transfer session
    if(mfiCommandDataLen >= 3) {
        mfiStatus.sessionIDiPod = (pCommandData[0]<<8) | pCommandData[1];
        // pCommandData[2] = protocolIndex, which should match IPOD_ACCESSORY_EA_INDEX, but doesn't really matter
        // Send ack
        MFISendAck(IPOD_OpenDataSessionForProtocol, IPOD_ACK_SUCCESS);
    }
    else {
        // Not enough parameters
        MFISendAck(IPOD_OpenDataSessionForProtocol, IPOD_ACK_BAD_PARAMETER);
    }
}


void MFIHandleDataTransfer(void)
{
    uint8_t commandAckPayloadSize;
    uint8_t receivedRomoCommand;
    uint8_t pCommandDataIndex = 2;

    // Check the transaction ID to make sure we aren't handling a retransmitted packet
    if(mfiStatus.transactionIDiPod != mfiStatus.transactionIDiPodLastDataTransfer) {
        mfiStatus.transactionIDiPodLastDataTransfer = mfiStatus.transactionIDiPod;
        
        // Incoming data/command from app!
        // Bytes 0 and 1 form the 16-bit sessionID
        // One could (should?) check that here, but we probably don't care
        // Packets may include multiple commands -- look for additional sync bytes to handle
        mfiStatus.sessionIDiPod = (pCommandData[0]<<8) + pCommandData[1];
        
        // Ack the IAP packet so that is out of the way, and then process commands within it
        MFISendAck(IPOD_iPodDataTransfer, IPOD_ACK_SUCCESS);
        
        mfiAckPayloadSize = 2; // First 2 bytes are the session ID

        // Check data for Romo commands
        while(pCommandDataIndex < mfiCommandDataLen) {
            receivedRomoCommand = pCommandData[pCommandDataIndex];
            pCommandDataIndex += ProcessCommand(pCommandData+pCommandDataIndex, &commandAckPayloadSize, mfiAckBuf+mfiAckPayloadSize);
            
            // These two commands require special iAP packets, so we set flags for them to be sent
            // in subsequent calls to this function
            if(receivedRomoCommand == CMD_SET_DEV_CHARGE_CURRENT) {
                mfiStatus.communication.updateChargeCurrent = TRUE;
            }
            else if(receivedRomoCommand == CMD_SET_DEV_CHARGE_ENABLE) {
                mfiStatus.communication.updateChargeState = TRUE;
            }

            mfiAckPayloadSize += commandAckPayloadSize;
        }
        
        mfiAckBuf[0] = mfiStatus.sessionIDiPod>>8;
        mfiAckBuf[1] = mfiStatus.sessionIDiPod;
        
        MFIQueueCommand(IPOD_AccessoryDataTransfer, mfiAckBuf, mfiAckPayloadSize);
    }
}


void MFIHandleRetiPodOptionsForLingo(void)
{    
    //mfiIdentStatus.acked.getiPodOptionsForLingo = TRUE;
    if(mfiIdentStatus.transIDs.getiPodOptionsForLingo == mfiStatus.transactionIDiPod) {
        mfiIdentStatus.acked.getiPodOptionsForLingo = TRUE;
        if(pCommandData[0] == IPOD_LINGO_GENERAL) {
            // Bytes 1 and 2 of the 8-byte iPod Options contain relevant info
            if(!BitIsSet(pCommandData[7], 5)) {
                // Does not support communication with apps.  End IDPS and fail.
                // Send EndIDPS with status 0x02 (abandoning IDPS)
                mfiAckBuf[0] = 0x02;
                MFIQueueCommand(IPOD_EndIDPS, mfiAckBuf, 1);
                ledControl.mode = RMLedModeBlink;
                LEDSetBlink(200,100);
                while(1);
            }
            else if (!BitIsSet(pCommandData[6],7)) {
                // Does not support eaProtocolMetadataToken
                // Set it as acked to skip
                mfiIdentStatus.fidTokensAcked.eaProtocolMetadataToken = TRUE;
            }
        }
        mfiStatus.flags.waitingForAck = FALSE;
    }
}


void MFITasks(void)
{
    mfiCommand = 0xFF;        // Initialize to non-existant command
    mfiCommandDataLen = 0;
    mfiAckPayloadSize = 0;
    memset(mfiAckBuf, 0, CMD_MAX_PAYLOAD+4);

    if(mfiStatus.flags.fatalError) {
        LEDSOS();
    }
    
    #ifdef SEND_STARTUP
    if(!mfiStatus.communication.sentStartup) {
        mfiAckBuf[0] = HIGH_BYTE(mfiStatus.sessionIDiPod);
        mfiAckBuf[1] = LOW_BYTE(mfiStatus.sessionIDiPod);
        mfiAckBuf[2] = RMCommandFromRobotAsyncEvent;
        mfiAckBuf[3] = 1; //payload length
        mfiAckBuf[4] = RMAsyncEventTypeStartup;

        MFIQueueCommand(IPOD_AccessoryDataTransfer, mfiAckBuf, 5);
        mfiStatus.communication.sentStartup = TRUE;
    }
    #endif //SEND_STARTUP
    
    if(mfiStatus.flags.waitingForAck) {
        // Check for ack timeout
        if(ackWaitCnt >= mfiStatus.ackWaitTicks) {	// Check for ack timeout
            if(!mfiStatus.communication.isAuthenticating && !mfiStatus.communication.isIdentifying) {
                if(mfiResendCount++ < MFI_MAX_RETRIES) {
                    if(mfiPendingPacket.commandID) {
                        MFISendQueuedCommand();
                    }
                    ackWaitCnt = 0;
                }
            }
            mfiStatus.flags.waitingForAck = FALSE;	// Retries exceeded
            
            if(mfiStatus.communication.isIdentifying && !mfiIdentStatus.acked.startIDPS) {
                mfiStatus.flags.needsIDPS = TRUE;
            }
        }
    }
    
    // Send special commands for setting the iDevice charging state and current,
    // if the app has requested it
    MFIChargingTasks();

    // Read a command from the input buffer
    MFIGetCommand();
    //    MFIGetCommandFSM();


    if(mfiStatus.flags.newPacketReceived) {
        // Kick the watchdog every time we get any sort of valid packet
        wdt_reset();
        
        #if 1 //used for code folding
        switch(mfiCommand) {
            #if 1 //code folding
            case IPOD_RequestIdentify:
            // Re-identify by sending StartIDPS
            mfiStatus.flags.needsIDPS = TRUE;
            break;

            case IPOD_iPodAck:
            MFIHandleAck();
            break;

            case IPOD_ReturnTransportMaxPayloadSize:
            // Store max payload size?  We're always using 160, which is less than the default of 506
            mfiStatus.flags.waitingForAck = FALSE;
            mfiIdentStatus.acked.requestTransportMaxPayloadSize = TRUE;
            break;

            case IPOD_GetAccessoryAuthenticationInfo:
            // iDevice is ready to start authentication
            mfiStatus.flags.needsAuthentication = TRUE;
            break;

            case IPOD_AckAccessoryAuthenticationInfo:
            MFIHandleAckAccessoryAuthenticationInfo();
            break;
            
            case IPOD_GetAccessoryAuthenticationSignature:
            // Includes 20 bytes of challenge data for Auth 2.0 and 1 byte for a retry counter
            if(mfiCommandDataLen >= 20) {
                MFIAuthSendSignature(pCommandData, 20);
            }
            break;

            case IPOD_AckAccessoryAuthenticationStatus:
            MFIHandleAckAccessoryAuthenticationStatus();
            break;

            case IPOD_AckFIDTokenValues:
            MFIHandleAckFIDTokens();
            break;

            case IPOD_IDPSStatus:
            MFIHandleIDPSStatus();
            break;

            case IPOD_OpenDataSessionForProtocol:
            MFIHandleOpenDataSessionForProtocol();
            break;

            case IPOD_CloseDataSession:
            // Terminate data transfer session and send ack
            mfiStatus.flags.waitingForAck = FALSE;
            mfiStatus.sessionIDiPod = 0;	// zero-out session ID
            MFISendAck(IPOD_CloseDataSession, IPOD_ACK_SUCCESS);
            break;

            case IPOD_iPodDataTransfer:
            MFIHandleDataTransfer();
            break;

            case IPOD_RetiPodOptionsForLingo:
            MFIHandleRetiPodOptionsForLingo();
            break;

            case IPOD_ReturnExtendedInterfaceMode:
            case IPOD_EnterRemoteUIMode:
            case IPOD_ReturniPodName:
            case IPOD_ReturniPodSoftwareVersion:
            case IPOD_ReturniPodSerialNum:
            case IPOD_ReturnLingoProtocolVersion:
            case IPOD_RetiPodAuthenticationInfo:
            case IPOD_RetiPodAuthenticationSignature:
            case IPOD_NotifyiPodStateChange:
            case IPOD_RetiPodOptions:
            case IPOD_GetAccessoryInfo:
            case IPOD_RetiPodPreferences:
            case IPOD_RetUIMode:
            case IPOD_SetAccessoryStatusNotification:
            case IPOD_iPodNotification:
            case IPOD_RetEventNotification:
            case IPOD_RetSupportedEventNotification:
            case IPOD_RetNowPlayingApplicationBundleName:
            case IPOD_RetLocalizationInfo:
            case IPOD_WiFiConnectionInfo:

            default:
            break;

            #endif
        }
        
        mfiStatus.flags.newPacketReceived = FALSE;
        #endif
    }
    MFILaunchApp();
}


// Queue the command for sending when ready
// Returns true if the command was placed in the queue
// Returns false if the command was not placed in the queue
BOOL MFIQueueCommand(uint8_t commandID, uint8_t *data, uint8_t dataLen)
{
    if(!mfiStatus.flags.waitingForAck) {
        mfiPendingPacket.commandID = commandID;
        mfiPendingPacket.dataLen = dataLen;
        mfiPendingPacket.transactionID = mfiStatus.transactionIDRomo++;
        memcpy((void *)mfiPendingPacket.data, data, dataLen);

        mfiStatus.flags.waitingForAck = TRUE;
        ackWaitCnt = 0;
        mfiResendCount = 0;
        return MFISendQueuedCommand();
    }
    else {
        return FALSE;
    }
}


// Send the currently queued command
BOOL MFISendQueuedCommand()
{
    return MFISendCommand(mfiPendingPacket.commandID, mfiPendingPacket.data, mfiPendingPacket.dataLen, mfiPendingPacket.transactionID);
}


// Send the mfi command out over the serial port
BOOL MFISendCommand(uint8_t commandID, volatile uint8_t *data, uint8_t dataLen, uint16_t transID)
{
    if(BytesFreeOutBuf() >= (dataLen + 9)) {
        int8_t checksum = 0;

        MutexLockOutBuf();
        PutByteOutBuf(SYNC_BYTE);			// Sync Byte
        PutByteOutBuf(START_BYTE);			// Start Byte
        PutByteOutBuf(dataLen+4);			// Length byte = LingoID(1) + CommandID(1) + TransID(2) + Data(n) = n+4
        PutByteOutBuf(IPOD_LINGO_GENERAL);	// LingoID
        PutByteOutBuf(commandID);			// CommandID (StartIDPS)
        PutByteOutBuf(HIGH_BYTE(transID));  // TransID high byte
        PutByteOutBuf(LOW_BYTE(transID));	// TransID low byte
        
        for(int i=0; i<dataLen; i++) {
            PutByteOutBuf(data[i]);			// Data
            checksum += (int8_t)data[i];
        }
        checksum += (int8_t)(dataLen+4) + (int8_t)IPOD_LINGO_GENERAL + (int8_t)commandID + (int8_t)(HIGH_BYTE(transID)) + (int8_t)(LOW_BYTE(transID));
        checksum ^= 0xFF;
        checksum += 0x01;

        PutByteOutBuf(checksum);            // Checksum
        MutexUnlockOutBuf();
        return TRUE;
    }
    else {
        return FALSE;
    }
}


// Send an MFI acknowledgement packet for the given commandID with the given result
BOOL MFISendAck(uint8_t commandID, uint8_t result)
{
    return MFISendCommand(IPOD_AccessoryACK, (uint8_t[]){result, commandID}, 2, mfiStatus.transactionIDiPod);
}

