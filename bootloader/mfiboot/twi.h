/*
* twi.h
*
* Created: 8/20/2012 5:59:45 PM
*  Author: Dan Kane
*/


#ifndef TWI_H_
#define TWI_H_

#include <util/twi.h>
#include "SerialProtocol.h"

#define IPOD_CP_FREQUENCY_LOW                   (40000ul)


//////////////////////////////////////////////////////////////////////////
// TWI Status/Control register definitions
//////////////////////////////////////////////////////////////////////////

#define TWI_BUFFER_SIZE				CMD_MAX_PAYLOAD+1     // Set this to the largest message size that
// will be sent including address byte.
#define TWI_TWBR(freq)				(((F_CPU/freq)-16)/2)			// Calculate TWI Bit rate Register setting.
#define TWI_SET_FREQUENCY(freq)		TWBR = TWI_TWBR(freq)			// Set TWI baudrate



register uint8_t twiState asm("r2");
extern uint8_t TWI_buf[TWI_BUFFER_SIZE];			// Transceiver buffer
extern uint8_t TWI_lastTransOK;
extern uint8_t TWI_msgSize;


//////////////////////////////////////////////////////////////////////////
// Function definitions
//////////////////////////////////////////////////////////////////////////

#define TWI_TRANSCEIVER_BUSY		bit_is_set(TWCR,TWIE)
void TWIStartTransceiverWithData(uint8_t *, uint8_t);
void TWIStartTransceiver(void);
void TWIGetDataFromTransceiver(uint8_t *, uint8_t);
void TWISend(uint8_t *msg, uint8_t dataSize);
void CPWrite(uint8_t addr, uint8_t *data, uint8_t dataSize);
void CPRead(uint8_t addr, uint8_t *data, uint8_t dataSize);



//////////////////////////////////////////////////////////////////////////
// Bit and byte definitions
//////////////////////////////////////////////////////////////////////////

#define TWI_READ_BIT  0       // Bit position for R/W bit in "address byte".
#define TWI_ADR_BITS  1       // Bit position for LSB of the slave address bits in the init byte.

//////////////////////////////////////////////////////////////////////////
// TWI State codes
//////////////////////////////////////////////////////////////////////////

// General TWI Master status codes
#define TWI_START                  0x08  // START has been transmitted
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter status codes
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received

// TWI Master Receiver status codes
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter status codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received

// TWI Slave Receiver status codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = “0”
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition


#endif /* TWI_H_ */