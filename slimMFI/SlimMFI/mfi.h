/*
* mfi_commands.h
*
* Created: 9/18/2012 6:53:28 PM
*  Author: Dan Kane
*/


#ifndef MFI_COMMANDS_H_
#define MFI_COMMANDS_H_

#include "romo3.h"
#include "mfi_config.h"
#include "mfi_auth.h"
#include "mfi_ident.h"
#include "util.h"

struct mfiStatus_t {
    volatile uint16_t transactionIDiPod;
    volatile uint16_t transactionIDRomo;
    volatile uint16_t sessionIDiPod;
    
    uint32_t ackWaitTicks;
    
    union {
        uint8_t all;
        struct {
            uint8_t needsIDPS			: 1;
            uint8_t needsAuthentication	: 1;
            uint8_t newCommandReceived	: 1;
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
        };
    } communication;
};

extern volatile struct mfiStatus_t mfiStatus;


#define SYNC_BYTE									0xFF
#define SYNC_BYTE_1									0x5A
#define SYNC_BYTE_2									0xA5
#define START_BYTE									0x55

//////////////////////////////////////////////////////////////////////////
// General Lingo Commands
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Incoming Commands

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

void MFIInit();
void MFISendCommand(uint8_t commandID, uint8_t *data, uint8_t dataLen, volatile uint16_t *transID);
BOOL MFIReceiveCommand(uint8_t *command, uint8_t *commandData, uint8_t *commandDataLen);
#endif /* MFI_COMMANDS_H_ */