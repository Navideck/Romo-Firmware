/*
* mfi_ident.h
*
* Created: 9/20/2012 4:57:47 PM
*  Author: dkane
*/


#ifndef MFI_IDENT_H_
#define MFI_IDENT_H_

#include "romo3.h"
#include "util.h"

struct mfiIdentStatus_t {
    struct {
        uint16_t startIDPS;
        uint16_t requestTransportMaxPayloadSize;
        uint16_t getiPodOptionsForLingo;
        uint16_t setFIDTokenValues;
        uint16_t endIDPS;
    } transIDs;
    
//     union {
//         uint8_t all;
//         struct {
//             uint8_t startIDPS						: 1;
//             uint8_t requestTransportMaxPayloadSize	: 1;
//             uint8_t getiPodOptionsForLingo			: 1;
//             uint8_t setFIDTokenValues				: 1;
//             uint8_t endIDPS							: 1;
//         };
//     } acked;
    
//     union {
//         uint16_t all;								// Bit positions match token subtype
//         struct {
//             uint8_t identifyToken			: 1;	// bit 0
//             uint8_t accessoryCapsToken		: 1;	// bit 1
//             uint8_t accessoryInfoTokens		: 1;	// bit 2
//             uint8_t iPodPreferencesToken	: 1;	// bit 3 NOT USED
//             uint8_t eaProtocolToken			: 1;	// bit 4
//             uint8_t bundleSeedIDPrefToken	: 1;	// bit 5
//             uint8_t skipped					: 2;	// bits 6,7	NOT USED
//             uint8_t eaProtocolMetadataToken	: 1;	// bit 8
//         };
//     } fidTokensAcked;
    
//     union {
//         uint16_t all;								// Bit positions match token info type
//         struct {
//             uint8_t reserved0				: 1;	// bit 0 NOT USED
//             uint8_t name					: 1;	// bit 1
//             uint8_t reserved2_3				: 2;	// bits 2,3 NOT USED
//             uint8_t firmwareVersion			: 1;	// bit 4
//             uint8_t hardwareVersion			: 1;	// bit 5
//             uint8_t manufacturer			: 1;	// bit 6
//             uint8_t modelNumber				: 1;	// bit 7
//             uint8_t serialNumber			: 1;	// bit 8
//             uint8_t maxPayloadSize			: 1;	// bit 9
//             uint8_t reserved_10				: 1;	// bit 10 NOT USED
//             uint8_t accStatus				: 1;	// bit 11
//             uint8_t rfCertifications		: 1;	// bit 12
//         };
//     } fidInfoTokensSent;
};

extern volatile struct mfiIdentStatus_t mfiIdentStatus;


//////////////////////////////////////////////////////////////////////////
// FID Tokens
//////////////////////////////////////////////////////////////////////////

//#define FID_REQUIRED_TOKEN_MASK				0x0137	// bits specified below
#define FID_TYPE_0							0x00
#define FID_IDENTIFY_TOKEN					0x00
#define FID_ACCESSORY_CAPS_TOKEN			0x01
#define FID_ACCESSORY_INFO_TOKEN			0x02
//#define FID_IPOD_PREFERENCE_TOKEN			0x03	// NOT NEEDED (removed from mask)
#define FID_EA_PROTOCOL_TOKEN				0x04
#define FID_BUNDLE_SEED_ID_PREF_TOKEN		0x05
#define FID_EA_PROTOCOL_METADATA_TOKEN		0x08

//#define FID_REQ_ACC_INFO_TOKEN_MASK			0x1BF2	// bits specified below
#define FID_ACC_INFO_NAME					0x01
#define FID_ACC_INFO_FIRMWARE_VERSION		0x04
#define FID_ACC_INFO_HARDWARE_VERSION		0x05
#define FID_ACC_INFO_MANUFACTURER			0x06
#define FID_ACC_INFO_MODEL_NUMBER			0x07
#define FID_ACC_INFO_SERIAL_NUMBER			0x08
#define FID_ACC_INFO_MAX_PAYLOAD_SIZE		0x09
#define FID_ACC_INFO_STATUS					0x0B
#define FID_ACC_INFO_RF_CERTIFICATIONS		0x0C

//////////////////////////////////////////////////////////////////////////
// MFI Identification Functions
//////////////////////////////////////////////////////////////////////////

void MFIIdentInit( void );
BOOL MFIIdentify( void );
uint8_t MFIIdentSendFIDTokens( uint8_t firstToken );

#endif /* MFI_IDENT_H_ */