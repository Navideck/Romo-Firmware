/*
* mfi_config.h
*
* Created: 8/20/2012 7:29:54 PM
*  Author: Dan Kane
*/

#ifndef MFI_CONFIG_H_
#define MFI_CONFIG_H_

#define _MFI_CONFIG_VERSION_MAJOR 3
#define _MFI_CONFIG_VERSION_MINOR 4
#define _MFI_CONFIG_VERSION_DOT   0
#define _MFI_CONFIG_VERSION_BUILD 0

#define MFI_MAX_RETRIES                         3


//------------------------------------------------------------------------------
// iPod Communication Interface

#define IPOD_COMMAND_PAYLOAD_SIZE				160
#define IPOD_OPERATIONAL_PARAMETERS_BYTE0       0x00
#define IPOD_OPERATIONAL_PARAMETERS_BYTE1       0x60
#define IPOD_OPERATIONAL_PARAMETERS_BYTE2       0x04
#define IPOD_CP_INTERFACE                       0x22			//0X22 ON PRODUCTION BOARDS
#define IPOD_CP_VERSION_1_0						0X01
#define IPOD_CP_VERSION_2_0A					0X02
#define IPOD_CP_VERSION_2_0B					0X03
#define IPOD_CP_VERSION_2_0C					0X05
#define IPOD_CP_FREQUENCY_LOW                   (40000ul)
#define IPOD_CP_FREQUENCY_HIGH                  (400000ul)
#define IPOD_COMMAND_DELAY_IDL                  0
#define IPOD_COMMAND_DELAY                      0

//------------------------------------------------------------------------------
// Accessory Information

#define IPOD_ACCESSORY_NAME                     "Romo"						// 64 char max
#define IPOD_ACCESSORY_MANUFACTURER             "Romotive, Inc"				// 64 char max
#define IPOD_ACCESSORY_MODEL_NUMBER             "3a"
#define IPOD_ACCESSORY_SERIAL_NUMBER            "000000000000000001"


//------------------------------------------------------------------------------
// Lingoes, Capabilities, and Preferences

#define IPOD_LINGO_GENERAL                      0x00    // General lingo identifier

#define IPOD_ACCESSORY_PREFERENCES_BYTE0        0x00
#define IPOD_ACCESSORY_PREFERENCES_BYTE1        0x00

#define IPOD_ACCESSORY_CAPABILITIES_BYTE0       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE1       0x02
#define IPOD_ACCESSORY_CAPABILITIES_BYTE2       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE3       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE4       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE5       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE6       0x00
#define IPOD_ACCESSORY_CAPABILITIES_BYTE7       0x00


//------------------------------------------------------------------------------
// Accessory RF Certifications

#define IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE0  0x00
#define IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE1  0x00
#define IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE2  0x00
#define IPOD_ACCESSORY_RF_CERTIFICATIONS_BYTE3  0x00


//------------------------------------------------------------------------------
// Device Options

#define IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE0     0x02
#define IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE1     0x00
#define IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE2     0x00
#define IPOD_ACCESSORY_DEVICE_OPTIONS_BYTE3     0x00


//------------------------------------------------------------------------------
// OS Application Information

#define IPOD_ACCESSORY_BUNDLE_SEED_ID           "UW37QL9ULT"
#define IPOD_ACCESSORY_BUNDLE_ID                "com.romotive.romo"     // Or .RomoTest?

/*
#define IPOD_ACCESSORY_EA_STRINGS				"\x01" "com.romotive.Romo" "\0" \
"\x01" "com.romotive.RomoTest"
*/
#define IPOD_ACCESSORY_EA_INDEX					0x01
#define IPOD_ACCESSORY_EA_STRING				"com.romotive.romo"


//------------------------------------------------------------------------------
// Extra USB Charging Current (USB only)

#define IPOD_USB_EXTRA_CURRENT_IN_SUSPEND       0
#define IPOD_USB_EXTRA_CURRENT_NOT_IN_SUSPEND   500


#endif /* MFI_CONFIG_H_ */

