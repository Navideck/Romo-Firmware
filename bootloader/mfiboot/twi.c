/*
 * twi.c
 *
 * Created: 8/16/2013 11:12:26 AM
 *  Author: aarons
 */ 

#include "twi.h"
#include "mfiboot.h"
#include "util.h"
#include <avr/wdt.h>

uint8_t TWI_buf[TWI_BUFFER_SIZE];			// Transceiver buffer
uint8_t TWI_lastTransOK;
uint8_t TWI_msgSize;

void TWISend(uint8_t *msg, uint8_t dataSize)
{
    TWI_msgSize = dataSize;
    memcpy(TWI_buf, msg, dataSize);

    TWIStartTransceiver();
    while(twiState == TWI_MTX_ADR_NACK || twiState == TWI_MRX_ADR_NACK) {
        TWIStartTransceiver();				// Try again
    }
}


void CPWrite(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
    uint8_t msg[dataSize+2];
    msg[0] = IPOD_CP_INTERFACE & ~_BV(TWI_READ_BIT);			// First byte is the CP slave write address
    msg[1] = addr;												// Second byte is register address at which to begin writing

    memcpy(msg+2, data, dataSize);					    		// Copy data from data to msg starting at [2]

    TWISend(msg, dataSize+2);
}


void CPRead(uint8_t addr, uint8_t *data, uint8_t dataSize)
{
    CPWrite(addr, data, 0);										// First send write sequence with no data
    uint8_t msg[dataSize+1];									// Include one extra byte for the address

    msg[0] = IPOD_CP_INTERFACE | _BV(TWI_READ_BIT);				// Set the CP slave read address

    TWISend(msg, dataSize+1);
    
    memcpy(data, TWI_buf+1, dataSize);
}


void TWIStartTransceiver(void)
{
    static uint8_t TWI_bufPtr;
    uint8_t twi_finished = FALSE;

    TWI_lastTransOK = FALSE;
    twiState = TWI_NO_STATE;					    // Reset TWI_state.
    TWCR =	(1<<TWEN)|							    // TWI Interface enabled.
    (0<<TWIE)|(1<<TWINT)|				            // Disable TWI Interrupt and clear the flag.
    (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|		        // Initiate a START condition.
    (0<<TWWC);								        //
    
    while(!twi_finished) {
        // Wait for TWINT flag
        do {
            wdt_reset();
        } while(!bit_is_set(TWCR, TWINT));

        switch(TWSR) {
            case TWI_START:							    // START has been transmitted
            case TWI_REP_START:						    // Repeated START has been transmitted
            TWI_bufPtr = 0;							    // Set buffer pointer to the TWI Address location
            case TWI_MTX_ADR_ACK:					    // SLA+W has been transmitted and ACK received
            case TWI_MTX_DATA_ACK:					    // Data byte has been transmitted and ACK received
            if(TWI_bufPtr < TWI_msgSize) {
                TWDR = TWI_buf[TWI_bufPtr++];
                TWCR = (1<<TWEN)|					    // TWI Interface enabled
                (0<<TWIE)|(1<<TWINT)|				    // Clear the flag to send byte
                (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	    //
                (0<<TWWC);							    //
            }
            else {								    // Send STOP after last byte
                TWI_lastTransOK = TRUE;				    // Set status bits to completed successfully.
                TWCR = (1<<TWEN)|					    // TWI Interface enabled
                (0<<TWIE)|(1<<TWINT)|				    // Disable TWI Interrupt and clear the flag
                (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|	    // Initiate a STOP condition.
                (0<<TWWC);							    //
                twi_finished = TRUE;
            }
            break;

            case TWI_MRX_DATA_ACK:					    // Data byte has been received and ACK transmitted
            TWI_buf[TWI_bufPtr++] = TWDR;
            case TWI_MRX_ADR_ACK:					    // SLA+R has been transmitted and ACK received
            if(TWI_bufPtr < (TWI_msgSize-1)) {		    // Detect the last byte to NACK it.
                TWCR = (1<<TWEN)|						// TWI Interface enabled
                (0<<TWIE)|(1<<TWINT)|				    // Clear the flag to read next byte
                (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	    // Send ACK after reception
                (0<<TWWC);							    //
            }
            else {							    // Send NACK after next reception
                TWCR = (1<<TWEN)|					    // TWI Interface enabled
                (0<<TWIE)|(1<<TWINT)|				    // Clear the flag to read next byte
                (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	    // Send NACK after reception
                (0<<TWWC);							    //
            }
            break;

            case TWI_MRX_DATA_NACK:					    // Data byte has been received and NACK transmitted
            TWI_buf[TWI_bufPtr] = TWDR;
            TWI_lastTransOK = TRUE;					    // Set status bits to completed successfully.
            TWCR = (1<<TWEN)|						    // TWI Interface enabled
            (0<<TWIE)|(1<<TWINT)|					    // Disable TWI Interrupt and clear the flag
            (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|		    // Initiate a STOP condition.
            (0<<TWWC);								    //
            twi_finished = TRUE;
            break;

            case TWI_MTX_ADR_NACK:					    // SLA+W has been transmitted and NACK received
            case TWI_MRX_ADR_NACK:					    // SLA+R has been transmitted and NACK received
            case TWI_MTX_DATA_NACK:					    // Data byte has been transmitted and NACK received
            //    case TWI_NO_STATE					    // No relevant state information available; TWINT = ?0?
            case TWI_BUS_ERROR:						    // Bus error due to an illegal START or STOP condition
            default:
            twiState = TWSR;						    // Store TWSR and automatically sets clears noErrors bit.
            // Reset TWI Interface
            TWCR =	(1<<TWEN)|						    // Enable TWI-interface and release TWI pins
            (0<<TWIE)|(0<<TWINT)|				        // Disable Interrupt
            (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	        // No Signal requests
            (0<<TWWC);							        //
            twi_finished = TRUE;
        }
    }
}