/*
* mfi_commands.h
*
* Created: 9/18/2012 6:53:28 PM
*  Author: Dan Kane
*/


#ifndef MFI_COMMANDS_H_
#define MFI_COMMANDS_H_

#include "main.h"
#include "mfi_auth.h"
#include "mfi_config.h"
#include "mfi_ident.h"
#include "util.h"
#include "pwm.h"
#include "ringbuffer.h"
#include "commands.h"
#include "uart.h"
#include <util/delay.h>

#ifndef MFI_REAUTH_IN_MAIN
#define MFI_REAUTH_IN_MAIN
#endif

typedef struct mfiStatus_ {
    volatile uint16_t transactionIDiPod;
    volatile uint16_t transactionIDRomo;
    volatile uint16_t sessionIDiPod;
    volatile uint16_t transactionIDiPodLastDataTransfer;
    
    uint32_t ackWaitTicks;
    
    union {
        uint8_t all;
        struct {
            uint8_t needsIDPS			: 1;
            uint8_t needsAuthentication	: 1;
            uint8_t newPacketReceived	: 1;
            uint8_t waitingForAck		: 1;
            uint8_t authComplete		: 1;
            uint8_t appLaunchRequested  : 1;
            uint8_t fatalError			: 1;
        };
    } flags;
    union {
        uint8_t all;
        struct {
            uint8_t isIdentifying		: 1;
            uint8_t isAuthenticating	: 1;
            uint8_t updateChargeCurrent : 1;
            uint8_t updateChargeState   : 1;
            uint8_t sentStartup         : 1;
        };
    } communication;
} mfiStatus_t;


typedef struct {
    uint16_t transactionID;
    uint8_t commandID;
    uint8_t dataLen;
    uint8_t data[CMD_MAX_PAYLOAD + 4];
} mfiPacket_t;


// Minimum packet size is StartByte(1), LengthByte(1), LingoID(1), CommandID(1), TransID(2), No Data(0), Checksum(1) = 7 bytes
typedef enum MFIReadState_t {
    MFI_READ_STATE_INIT,
    MFI_READ_STATE_GOT_SYNC,
    MFI_READ_STATE_GOT_START,
    MFI_READ_STATE_GOT_LENGTH,
    MFI_READ_STATE_GOT_LINGO,
    MFI_READ_STATE_GOT_COMMAND,
    MFI_READ_STATE_GOT_TRANS_ID,
    MFI_READ_STATE_GOT_PAYLOAD,
    MFI_READ_STATE_GOT_CHECKSUM,
    MFI_READ_STATE_CHECKSUM_PASSED,
    MFI_READ_STATE_CHECKSUM_FAILED
} MFIReadState;


extern volatile mfiStatus_t mfiStatus;
extern volatile mfiPacket_t mfiPendingPacket;
extern uint8_t mfiCommand;        // Initialize to non-existant command
extern uint8_t mfiCommandDataLen;
extern uint8_t mfiAckPayloadSize;
extern uint8_t mfiAckBuf[CMD_MAX_PAYLOAD + 4];
extern uint8_t mfiResendCount;
extern MFIReadState mfiReadState;



#define SYNC_BYTE									0xFF
#define SYNC_BYTE_1									0x5A
#define SYNC_BYTE_2									0xA5
#define START_BYTE									0x55

//////////////////////////////////////////////////////////////////////////
// General Lingo Commands
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Incoming Commands
//////////////////////////////////////////////////////////////////////////

#define IPOD_RequestIdentify                        0x00
#define IPOD_iPodAck                                0x02
#define IPOD_ReturnExtendedInterfaceMode            0x04
#define IPOD_EnterRemoteUIMode                      0x05	//Deprecated in R44, but needed for older devices.
#define IPOD_ReturniPodName                         0x08
#define IPOD_ReturniPodSoftwareVersion              0x0A
#define IPOD_ReturniPodSerialNum                    0x0C
#define IPOD_ReturnLingoProtocolVersion             0x10
#define IPOD_ReturnTransportMaxPayloadSize          0x12
#define IPOD_GetAccessoryAuthenticationInfo         0x14
#define IPOD_AckAccessoryAuthenticationInfo         0x16
#define IPOD_GetAccessoryAuthenticationSignature    0x17
#define IPOD_AckAccessoryAuthenticationStatus       0x19
#define IPOD_RetiPodAuthenticationInfo              0x1B
#define IPOD_RetiPodAuthenticationSignature         0x1E
#define IPOD_NotifyiPodStateChange                  0x23
#define IPOD_RetiPodOptions                         0x25
#define IPOD_GetAccessoryInfo                       0x27
#define IPOD_RetiPodPreferences                     0x2A
#define IPOD_RetUIMode                              0x36
#define IPOD_AckFIDTokenValues                      0x3A
#define IPOD_IDPSStatus                             0x3C
#define IPOD_OpenDataSessionForProtocol             0x3F
#define IPOD_CloseDataSession                       0x40
#define IPOD_iPodDataTransfer                       0x43
#define IPOD_SetAccessoryStatusNotification         0x46
#define IPOD_iPodNotification                       0x4A
#define IPOD_RetiPodOptionsForLingo                 0x4C
#define IPOD_RetEventNotification                   0x4E
#define IPOD_RetSupportedEventNotification          0x51
#define IPOD_RetNowPlayingApplicationBundleName     0x66
#define IPOD_RetLocalizationInfo                    0x68
#define IPOD_WiFiConnectionInfo                     0x6A

//////////////////////////////////////////////////////////////////////////
// Outgoing Commands
//////////////////////////////////////////////////////////////////////////

#define IPOD_RequestExtendedInterfaceMode           0x03
#define IPOD_ExitExtendedInterfaceMode              0x06
#define IPOD_RequestiPodName                        0x07
#define IPOD_RequestiPodSoftwareVersion             0x09
#define IPOD_RequestiPodSerialNum                   0x0B
#define IPOD_RequestLingoProtocolVersion            0x0F
#define IPOD_RequestTransportMaxPayloadSize         0x11
#define IPOD_IdentifyDeviceLingoes                  0x13
#define IPOD_RetAccessoryAuthenticationInfo         0x15
#define IPOD_RetAccessoryAuthenticationSignature    0x18
#define IPOD_GetiPodAuthenticationInfo              0x1A
#define IPOD_AckiPodAuthenticationInfo              0x1C
#define IPOD_GetiPodAuthenticationSignature         0x1D
#define IPOD_AckiPodAuthenticationStatus            0x1F
#define IPOD_GetiPodOptions                         0x24
#define IPOD_RetAccessoryInfo                       0x28
#define IPOD_GetiPodPreferences                     0x29
#define IPOD_SetiPodPreferences                     0x2B
#define IPOD_GetUIMode                              0x35
#define IPOD_SetUIMode                              0x37
#define IPOD_StartIDPS                              0x38
#define IPOD_SetFIDTokenValues                      0x39
#define IPOD_EndIDPS                                0x3B
#define IPOD_AccessoryACK                           0x41
#define IPOD_AccessoryDataTransfer                  0x42
#define IPOD_RetAccessoryStatusNotification         0x47
#define IPOD_AccessoryStatusNotification            0x48
#define IPOD_SetEventNotification                   0x49
#define IPOD_GetiPodOptionsForLingo                 0x4B
#define IPOD_GetEventNotification                   0x4D
#define IPOD_GetSupportedEventNotification          0x4F
#define IPOD_CancelCommand                          0x50
#define IPOD_SetAvailableCurrent                    0x54
#define IPOD_SetInternalBatteryChargingState        0x56
#define IPOD_RequestApplicationLaunch               0x64
#define IPOD_GetNowPlayingApplicationBundleName     0x65
#define IPOD_GetLocalizationInfo                    0x67
#define IPOD_RequestWiFiConnectionInfo              0x69


//////////////////////////////////////////////////////////////////////////
// ACK Constants
//////////////////////////////////////////////////////////////////////////

#define IPOD_ACK_SUCCESS                                0x00
#define IPOD_ACK_UNKNOWN_DATABASE_CATEGORY              0x01
#define IPOD_ACK_COMMAND_FAILED                         0x02
#define IPOD_ACK_OUT_OF_RESOURCES                       0x03
#define IPOD_ACK_BAD_PARAMETER                          0x04
#define IPOD_ACK_UNKNOWN_ID                             0x05
#define IPOD_ACK_COMMAND_PENDING                        0x06
#define IPOD_ACK_NOT_AUTHENTICATED                      0x07
#define IPOD_ACK_BAD_AUTHENTICATION_VERSION             0x08
#define IPOD_ACK_ACCESSORY_POWER_MODE_REQUEST_FAILED    0x09
#define IPOD_ACK_CERTIFICATE_INVALID                    0x0A
#define IPOD_ACK_CERTIFICATE_PERMISSIONS_INVALID        0x0B
#define IPOD_ACK_FILE_IS_IN_USE                         0x0C
#define IPOD_ACK_INVALID_FILE_HANDLE                    0x0D
#define IPOD_ACK_DIRECTORY_NOT_EMPTY                    0x0E
#define IPOD_ACK_OPERATION_TIMED_OUT                    0X0F
#define IPOD_ACK_COMMAND_UNAVAILABLE                    0x10
#define IPOD_ACK_INVALID_ACCESSORY_RESISTOR             0x11
#define IPOD_ACK_ACCESSORY_NOT_GROUNDED                 0x12
#define IPOD_ACK_SUCCESS_MULTISECTION_DATA              0x13
#define IPOD_ACK_MAXIMUM_ACCESSORY_CONNECTIONS			0x15
#define IPOD_ACK_SessionWriteFailure                    0x17
#define IPOD_ACK_IPOD_OUT_MODE_ENTRY_ERROR              0x18

//////////////////////////////////////////////////////////////////////////
// MFI Functions
//////////////////////////////////////////////////////////////////////////

BOOL MFIInit(void);
void MFITasks(void);
BOOL MFISendAck(uint8_t commandID, uint8_t result);
BOOL MFIQueueCommand(uint8_t commandID, uint8_t *data, uint8_t dataLen);
BOOL MFISendQueuedCommand(void);
BOOL MFISendCommand(uint8_t commandID, volatile uint8_t *data, uint8_t dataLen, uint16_t transID);


static inline void MFIChargingTasks(void)
{
    if(mfiStatus.communication.updateChargeCurrent &&
    !mfiStatus.flags.waitingForAck &&
    !mfiStatus.flags.needsIDPS) {
        mfiAckBuf[0] = chargeInfo.iDeviceChargingCurrent >> 8;
        mfiAckBuf[1] = chargeInfo.iDeviceChargingCurrent;
        mfiStatus.communication.updateChargeCurrent = FALSE;
        MFIQueueCommand(IPOD_SetAvailableCurrent, mfiAckBuf, 2);
        mfiStatus.flags.waitingForAck = TRUE;
    }

    if(mfiStatus.communication.updateChargeState &&
    !mfiStatus.flags.waitingForAck &&
    !mfiStatus.flags.needsIDPS) {
        mfiAckBuf[0] = chargeInfo.iDeviceChargerState;
        mfiStatus.communication.updateChargeState = FALSE;
        MFIQueueCommand(IPOD_SetInternalBatteryChargingState, mfiAckBuf, 1);
        mfiStatus.flags.waitingForAck = TRUE;
    }
}


static inline void MFIGetCommand(void)
{
    // Reset newPacketReceived
    mfiStatus.flags.newPacketReceived = FALSE;

    while(BytesUsedInBuf() && (PeekByte(&inBuf, 0) != START_BYTE)) {
        if(MutexTryLockInBuf()) {
            GetByteInBuf();
            MutexUnlockInBuf();
        }
        else {
            _delay_us(10);
        }
    }

    uint8_t length = PeekByte(&inBuf, 1);

    // Then check if there are enough bytes (minimum) for a full packet
    // Minimum packet size is StartByte(1), LengthByte(1), LingoID(1), CommandID(1), TransID(2), No Data(0), Checksum(1) = 7 bytes
    //if(BytesUsedInBuf() >= 7) {
    // Length = LingoID(1) + CommandID(1) + TransID(2) + Data(n) = n + 4 bytes
    // Length + StartByte(1) + LengthByte(1) + Checksum(1) = length + 3 = all bytes in packet
    
    // Make sure we have all the bytes in the packet
    if((BytesUsedInBuf() >= length+3) && (length <= IPOD_COMMAND_PAYLOAD_SIZE)) {
        // Length>Max is an error condition. Enter the packet and let checksum fail)
        int8_t checksum = -1;
        int8_t checksumShouldBe = 0;
        #if 1 //code folding

        mfiCommand = PeekByte(&inBuf, 3); // CommandID
        
        uint8_t transID_H = PeekByte(&inBuf, 4); // Transaction ID high byte
        uint8_t transID_L = PeekByte(&inBuf, 5); // Transaction ID low byte

        // Process data
        for(int i=6; i<length+2; i++) {
            pCommandData[i-6] = PeekByte(&inBuf, i); // Data
            checksumShouldBe += (int8_t)pCommandData[i-6];
        }
        checksum = PeekByte(&inBuf, length+2);

        // Finish checksum
        checksumShouldBe += (int8_t)length + (int8_t)IPOD_LINGO_GENERAL + (int8_t)mfiCommand + (int8_t)transID_H + (int8_t)transID_L;
        checksumShouldBe ^= 0xFF;
        checksumShouldBe += 0x01;
        
        // Compare the checksum to what it should be
        if(checksum == checksumShouldBe) {
            MutexLockInBuf();
            DropBytesInBuf(length+3);
            MutexUnlockInBuf();
            
            mfiCommandDataLen = length-4;
            mfiStatus.transactionIDiPod = (uint16_t)((transID_H<<8) | transID_L);
            mfiStatus.flags.newPacketReceived = TRUE; // Packet received in full! Ready to process!
        }
        else {
            //UART0_TX_WHEN_READY(0x3F);
            GetByteInBuf();
        }
        #endif
        
    }
    else {
        if(packetTimer > MS_TO_TICKS(100)) {
            FlushInBuf();
            //UART0_TX_WHEN_READY(0xDF);
        }
    }
}


static inline void MFIGetCommandFSM(void)
{
    uint8_t length = 0;
    int8_t checksum = -1;
    int8_t checksumShouldBe = 0;
    uint8_t transIDHigh = 0;
    uint8_t transIDLow = 0;
    mfiStatus.flags.newPacketReceived = FALSE;
    uint8_t mfiCommandDataLen = 0;

    while(BytesUsedInBuf()) {
        if(MutexTryLockInBuf()) {
            
            switch(mfiReadState) {
                case MFI_READ_STATE_INIT:
                if(GetByteInBuf() == SYNC_BYTE) {
                    mfiReadState = MFI_READ_STATE_GOT_SYNC;
                }
                break;

                case MFI_READ_STATE_GOT_SYNC:
                if(GetByteInBuf() == START_BYTE) {
                    mfiReadState = MFI_READ_STATE_GOT_START;
                }
                else {
                    mfiReadState = MFI_READ_STATE_INIT;
                }
                break;

                case MFI_READ_STATE_GOT_START:
                length = GetByteInBuf();
                mfiCommandDataLen = length-4;
                checksumShouldBe += length;
                if(length <= IPOD_COMMAND_PAYLOAD_SIZE) {
                    mfiReadState = MFI_READ_STATE_GOT_LENGTH;
                }
                else {
                    mfiReadState = MFI_READ_STATE_INIT;
                }
                break;

                case MFI_READ_STATE_GOT_LENGTH:
                checksumShouldBe += GetByteInBuf();
                mfiReadState = MFI_READ_STATE_GOT_LINGO;
                break;

                case MFI_READ_STATE_GOT_LINGO:
                mfiCommand = GetByteInBuf();
                checksumShouldBe += mfiCommand;
                mfiReadState = MFI_READ_STATE_GOT_COMMAND;
                break;

                case MFI_READ_STATE_GOT_COMMAND:
                if(BytesUsedInBuf() > 1) {
                    transIDHigh = GetByteInBuf();
                    transIDLow = GetByteInBuf();
                    mfiStatus.transactionIDiPod = (uint16_t)((transIDHigh << 8) + transIDLow);
                    checksumShouldBe += transIDHigh + transIDLow;
                    mfiReadState = MFI_READ_STATE_GOT_TRANS_ID;
                }
                break;

                case MFI_READ_STATE_GOT_TRANS_ID:
                for(int i=0; i<mfiCommandDataLen; i++) {
                    pCommandData[i] = GetByteInBuf();
                    checksumShouldBe += (int8_t)pCommandData[i];
                }
                mfiReadState = MFI_READ_STATE_GOT_PAYLOAD;
                break;

                case MFI_READ_STATE_GOT_PAYLOAD:
                checksum = GetByteInBuf();
                mfiReadState = MFI_READ_STATE_GOT_CHECKSUM;
                break;

                case MFI_READ_STATE_GOT_CHECKSUM:
                checksumShouldBe ^= 0xFF;
                checksumShouldBe += 0x01;
                
                if(checksum == checksumShouldBe) {
                    mfiReadState = MFI_READ_STATE_CHECKSUM_PASSED;
                }
                else {
                    mfiReadState = MFI_READ_STATE_CHECKSUM_FAILED;
                }
                break;

                case MFI_READ_STATE_CHECKSUM_PASSED:
                mfiStatus.flags.newPacketReceived = TRUE;
                mfiReadState = MFI_READ_STATE_INIT;
                return;
                
                case MFI_READ_STATE_CHECKSUM_FAILED:
                mfiReadState = MFI_READ_STATE_INIT;
                return;
                break;
            }
            MutexUnlockInBuf();
        }
        else {
            _delay_us(10);
        }
    }
}


static inline void MFILaunchApp(void)
{
    if(mfiStatus.flags.authComplete) {
        // Try launching app
        if(!mfiStatus.flags.appLaunchRequested) {
            mfiAckBuf[0] = 0x00; // Reserved
            mfiAckBuf[1] = 0x02; // First attempt
            mfiAckBuf[2] = 0x00; // Reserved
            char *idString = IPOD_ACCESSORY_BUNDLE_ID;
            uint8_t i = 0;
            while(idString[i] != '\0') {
                mfiAckBuf[3+i] = idString[i]; // Copy tokenString into data
                i++;
            }
            mfiAckBuf[3+i++] = '\0'; // Add back null terminator
            
            MFIQueueCommand(IPOD_RequestApplicationLaunch, mfiAckBuf, 3+i);
            mfiStatus.flags.appLaunchRequested = TRUE;
        }
    }
}

#endif /* MFI_COMMANDS_H_ */