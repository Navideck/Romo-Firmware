/*
* mfi_auth.h
*
* Created: 8/20/2012 7:30:08 PM
*  Author: Dan Kane
*/


#ifndef MFI_AUTH_H_
#define MFI_AUTH_H_

#include "romo3.h"
#include "util.h"

#define CP_SELFTEST_X509_BIT	7
#define CP_SELFTEST_PK_BIT		6
#define CP_STATUS_ERR_BIT		7
#define CP_STATUS_BITMASK		0x70
#define MFI_AUTH_MAX_RETRIES	2
#define MFI_AUTH_PAGE_SIZE		128ul

struct mfiAuthStatus_t {
    uint8_t protocolMajorVersion;
    uint8_t protocolMinorVersion;
    uint8_t sectionIndex;
    uint8_t lastSection;
    uint16_t certDataBytesLeft;
    
    struct {
        uint16_t retAccessoryAuthenticationInfo;
        uint16_t retAccessoryAuthenticationSignature;
    } transIDs;
    
    union {
        uint8_t all;
        struct {
            uint8_t retAccessoryAuthenticationInfo		: 1;
            uint8_t retAccessoryAuthenticationSignature	: 1;
        };
    } acked;
};

extern struct mfiAuthStatus_t mfiAuthStatus;

//////////////////////////////////////////////////////////////////////////
// MFI Authentication Functions
//////////////////////////////////////////////////////////////////////////

//#define MFI_IS_AUTHENTICATING		mfiStatus.communication.isAuthenticating

BOOL MFIAuthInit( void );
BOOL MFIAuthenticate( void );
//void MFIAuthSendSignature( uint8_t*, uint16_t );

//////////////////////////////////////////////////////////////////////////
// Apple Authentication Co-processor Functions
//////////////////////////////////////////////////////////////////////////

void CPInit( void );
BOOL CPWrite( uint8_t addr, uint8_t *data, uint8_t dataSize );
BOOL CPRead( uint8_t addr, uint8_t *data, uint8_t dataSize );

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

#endif /* MFI_AUTH_H_ */