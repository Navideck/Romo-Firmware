/*
* mfiboot.h
*
* Created: 8/16/2013 11:19:04 AM
*  Author: aarons
*/


#ifndef MFIBOOT_H_
#define MFIBOOT_H_

#include <inttypes.h>
#include <avr/io.h>
#include "mfi.h"
#include "SerialProtocol.h"


// Watchdog settings
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))

//romo3 defines
#define	cbi(sfr, bit)	    		(_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)		    	(_SFR_BYTE(sfr) |= _BV(bit))
#define ADC_WAIT_FOR_CONVERSION		loop_until_bit_is_set(ADCSRA, ADIF)
#define ADC_SET_CHAN(ch)			ADMUX = ch; sbi(ADMUX, REFS0)
#define ADC_START_CONVERSION		sbi(ADCSRA, ADSC)
#define LOW_VOLTAGE_THRESHOLD		74   // ~0.2V
#define HIGH_VOLTAGE_THRESHOLD		930  // ~3.0V
#define LIGHTNING_DETECT_LOW        745  // ~2.4V
#define LIGHTNING_DETECT_HIGH       930  // ~3V
#define LIGHTNING_TEST_DELTA		62   // ~.2V
#define IPOD_CP_INTERFACE           0x22
#define AUTH_ACC_CERT_DATA_LENGTH	0x30		// 128 bytes
#define AUTH_ACC_CERT_DATA_START	0x31		// 128 bytes
#define CP_SELFTEST_PK_BIT		    6
#define CP_STATUS_ERR_BIT	    	7
#define CP_STATUS_BITMASK		    0x70
#define MFI_AUTH_MAX_RETRIES	    2
#define MFI_AUTH_PAGE_SIZE		    128ul
#define LED_START_FLASHES           3
#define BAUD_RATE                   57600L
#define TIMEOUT_MS                  2000
#define CMD_ACK                     0x40
#define LED_DDR                     DDRB
#define LED_PORT                    PORTB
#define LED_PIN                     PINB
#define LED                         PINB0
#define MFI_TESTING                 0
//#define USE_MUTEX                   1 //This adds slightly over 100 bytes

//
// NRWW memory
//
// Addresses below NRWW (Non-Read-While-Write) can be programmed while
// continuing to run code from flash, slightly speeding up programming
// time.  Beware that Atmel data sheets specify this as a WORD address,
// while optiboot will be comparing against a 16-bit byte address.  This
// means that on a part with 128kB of memory, the upper part of the lower
// 64k will get NRWW processing as well, even though it doesn't need it.
// That's OK.  In fact, you can disable the overlapping processing for
// a part entirely by setting NRWWSTART to zero.  This reduces code
// space a bit, at the expense of being slightly slower, overall.
// RAMSTART should be self-explanatory.  It's bigger on parts with a
// lot of peripheral registers.


#if defined(__AVR_ATmega164A__) || defined(__AVR_ATmega164PA__)
#define NRWWSTART (0x1C00)
#elif defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324PA__)
#define NRWWSTART (0x3800)
#endif


typedef struct {
    uint8_t sentSyncs_f:1;
    uint8_t flashedLEDs_f:1;
    uint8_t loopTriggered_f:1;
    uint8_t usingiAP_f:1;
    uint8_t programming_f:1;
    uint8_t newCommand_f:1;
    uint8_t applicationLaunchRequested_f:1;
    uint8_t waitingForAck_f:1;
} bootloaderStatus_t;


typedef struct {
    uint8_t startIDPSRequested:1;
    uint8_t maxPayloadSizeRequested:1;
    uint8_t optionsForLingoRequested:1;
    uint8_t authenticationInfoRequested:1;
    uint8_t authenticationInfoAcked:1;
    uint8_t FIDTokensSent:1;
    uint8_t endIDPSRequested:1;
} iAPIDPSStatus_t;

// Use GPIOR2 to store these boolean flags to save code space
#define bootloaderStatus                    GPIOR2
#define sentSyncs                           ((volatile bootloaderStatus_t*)&GPIOR2)->sentSyncs_f
#define flashedLEDs                         ((volatile bootloaderStatus_t*)&GPIOR2)->flashedLEDs_f
#define loopTriggered                       ((volatile bootloaderStatus_t*)&GPIOR2)->loopTriggered_f
#define usingiAP                            ((volatile bootloaderStatus_t*)&GPIOR2)->usingiAP_f
#define programming                         ((volatile bootloaderStatus_t*)&GPIOR2)->programming_f
#define newCommand                          ((volatile bootloaderStatus_t*)&GPIOR2)->newCommand_f
#define applicationLaunchRequested          ((volatile bootloaderStatus_t*)&GPIOR2)->applicationLaunchRequested_f
#define waitingForAck                       ((volatile bootloaderStatus_t*)&GPIOR2)->waitingForAck_f


//
// Global Variables - most variables are global for code size
//
register uint8_t myIndex asm("r3");
register uint8_t outIndex asm("r4");
register uint8_t inIndex asm("r5");
register uint8_t trigger asm("r6");
register uint8_t intBufUsed asm("r7");
register uint8_t intBufIndex asm("r8");

volatile uint16_t intCnt0;

extern iAPIDPSStatus_t iAPIDPSStatus;

extern uint8_t bytesInSection;
extern uint8_t robotType;
extern uint16_t address;
extern uint16_t adcValue;
extern uint16_t previousADCValue;
extern uint16_t certDataBytesLeft;
extern uint8_t intBuf[64];
extern uint8_t data[CMD_MAX_PAYLOAD];


//
// Function Prototypes
//
// The main function is in init9, which removes the interrupt vector table
// we don't need. It is also 'naked', which means the compiler does not
// generate any entry or exit code itself.
int main(void);
void putch(uint8_t ch);
void putByte(uint8_t ch);
uint8_t getch(void);
uint8_t getByte(void);
void getNBytes(uint8_t cnt); /* "static inline" is a compiler hint to reduce code size */
void verifySpace();
void adcWaitForConversion(uint8_t ch);

void ReadSerialNumber(char *serial);
void reset() __attribute__ ((naked));
void appStart() __attribute__ ((naked));



#endif /* MFIBOOT_H_ */