/*
* mfi_commands.h
*
* Created: 9/18/2012 6:53:28 PM
*  Author: Dan Kane
*/


#ifndef MFI_COMMANDS_H_
#define MFI_COMMANDS_H_

#include "SerialProtocol.h"

#define SYNC_BYTE									0xFF
#define SYNC_BYTE_1									0x5A
#define SYNC_BYTE_2									0xA5
#define START_BYTE									0x55

//////////////////////////////////////////////////////////////////////////
// General Lingo Commands
//////////////////////////////////////////////////////////////////////////
#define IPOD_LINGO_GENERAL                          0x00    // General lingo identifier



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
// Apple Authentication Co-processor Registers
//////////////////////////////////////////////////////////////////////////

#define AUTH_DEVICE_VERSION							0x00		// 1 byte
#define AUTH_FIRMWARE_VERSION						0x01		// 1 byte
#define AUTH_PROTOCOL_MAJOR_VERSION					0x02		// 1 byte
#define AUTH_PROTOCOL_MINOR_VERSION					0x03		// 1 byte
#define AUTH_DEVICE_ID								0x04		// 4 bytes
#define AUTH_ERROR_CODE								0x05		// 1 byte
#define AUTH_CONTROL_STATUS							0x10		// 1 byte
#define AUTH_SIGNATURE_DATA_LENGTH					0x11		// 2 bytes
#define AUTH_SIGNATURE_DATA							0x12		// 128 bytes
#define AUTH_CHALLENGE_DATA_LENGTH					0x20		// 2 bytes
#define AUTH_CHALLENGE_DATA							0x21		// 20 bytes
#define AUTH_ACC_CERT_DATA_LENGTH					0x30		// 128 bytes
#define AUTH_ACC_CERT_DATA_START					0x31		// 128 bytes
// #define AUTH_ACC_CERT_DATA_1						0x32		// 128 bytes
// #define AUTH_ACC_CERT_DATA_2						0x33		// 128 bytes
// #define AUTH_ACC_CERT_DATA_3						0x34		// 128 bytes
// #define AUTH_ACC_CERT_DATA_4						0x35		// 128 bytes
// #define AUTH_ACC_CERT_DATA_5						0x36		// 128 bytes
// #define AUTH_ACC_CERT_DATA_6						0x37		// 128 bytes
// #define AUTH_ACC_CERT_DATA_7						0x38		// 128 bytes
// #define AUTH_ACC_CERT_DATA_8						0x39		// 128 bytes
// #define AUTH_ACC_CERT_DATA_9						0x3A		// 128 bytes
// #define AUTH_ACC_CERT_DATA_10						0x3B		// 128 bytes
// #define AUTH_ACC_CERT_DATA_11						0x3C		// 128 bytes
// #define AUTH_ACC_CERT_DATA_12						0x3D		// 128 bytes
// #define AUTH_ACC_CERT_DATA_13						0x3E		// 128 bytes
// #define AUTH_ACC_CERT_DATA_14						0x3F		// 128 bytes
#define AUTH_SELFTEST_CONTROL_STATUS				0x40		// 1 byte
// #define AUTH_IPOD_CERT_DATA_LENGTH					0x50		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_0						0x51		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_1						0x52		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_2						0x53		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_3						0x54		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_4						0x55		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_5						0x56		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_6						0x57		// 128 bytes
// #define AUTH_IPOD_CERT_DATA_7						0x58		// 128 bytes

//////////////////////////////////////////////////////////////////////////
// Authentication Ack Values
//////////////////////////////////////////////////////////////////////////

#define AUTH_STATUS_SUCCESS							0x00
#define AUTH_STATUS_CERT_TOO_LONG					0x04
#define AUTH_STATUS_UNSUPPORTED						0x08
#define AUTH_STATUS_CERT_INVALID					0x0A
#define AUTH_STATUS_BAD_PERMISSIONS					0x0B


//////////////////////////////////////////////////////////////////////////
// FID Tokens
//////////////////////////////////////////////////////////////////////////

#define FID_REQUIRED_TOKEN_MASK				0x0137	// bits specified below
#define FID_TYPE_0							0x00
#define FID_IDENTIFY_TOKEN					0x00
#define FID_ACCESSORY_CAPS_TOKEN			0x01
#define FID_ACCESSORY_INFO_TOKEN			0x02
#define FID_EA_PROTOCOL_TOKEN				0x04
#define FID_BUNDLE_SEED_ID_PREF_TOKEN		0x05
#define FID_EA_PROTOCOL_METADATA_TOKEN		0x08

#define FID_REQ_ACC_INFO_TOKEN_MASK			0x1BF2	// bits specified below
#define FID_ACC_INFO_NAME					0x01
#define FID_ACC_INFO_FIRMWARE_VERSION		0x04
#define FID_ACC_INFO_HARDWARE_VERSION		0x05
#define FID_ACC_INFO_MANUFACTURER			0x06
#define FID_ACC_INFO_MODEL_NUMBER			0x07
#define FID_ACC_INFO_SERIAL_NUMBER			0x08
#define FID_ACC_INFO_MAX_PAYLOAD_SIZE		0x09
#define FID_ACC_INFO_STATUS					0x0B
#define FID_ACC_INFO_RF_CERTIFICATIONS		0x0C


#define CP_SELFTEST_X509_BIT	            7
#define CP_SELFTEST_PK_BIT		            6
#define CP_STATUS_ERR_BIT		            7
#define CP_STATUS_BITMASK		            0x70
#define MFI_AUTH_MAX_RETRIES	            2
#define MFI_AUTH_PAGE_SIZE		            128ul
#define IPOD_ACCESSORY_BUNDLE_SEED_ID       "UW37QL9ULT"
#define IPOD_ACCESSORY_BUNDLE_ID            "com.romotive.romo"
#define IPOD_ACCESSORY_EA_INDEX				0x01
#define IPOD_ACCESSORY_EA_STRING			"com.romotive.romo"


register uint8_t mfiCommand asm("r9");
register uint8_t mfiCommandDataLen asm("r10");
register uint8_t sessionID asm("r11");

extern uint8_t pCommandData[CMD_MAX_PAYLOAD+9];
extern uint16_t transIDiPod;
extern uint16_t transIDRomo;


void MFISendAck(uint8_t cmd, uint8_t result);
void MFISendCommand(uint8_t cmd, uint8_t len);
void MFISendAuthData(uint8_t cmd, uint8_t len);
void MFIGetCommand(void);
void AddFIDTokenWithByte(uint8_t infoType, uint8_t b, char *buf, uint8_t length);
void AddFIDToken(uint8_t subType, char *buf, uint8_t length);
void AddFIDInfoToken(uint8_t infoType, char *buf, uint8_t length);



#endif //MFI_COMMANDS_H