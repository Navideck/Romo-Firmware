/*
* twi.c
*
* Created: 8/20/2012 5:59:57 PM
*  Author: Dan Kane
*/

#include "romo3.h"
#include "twi.h"
#include "mfi_config.h"
#include "util.h"

static uint8_t TWI_buf[ TWI_BUFFER_SIZE ];			// Transceiver buffer
static uint8_t TWI_msgSize;							// Number of bytes to be transmitted.
static uint8_t twiState = TWI_NO_STATE;			// State byte. Default set to TWI_NO_STATE.
static uint8_t TWI_lastTransOK = FALSE;

void TWIInit( void )
{
    TWI_SET_FREQUENCY(IPOD_CP_FREQUENCY_LOW);		// Initialize to low speed
    TWDR = 0xFF;                                    // Default content = SDA released.
    TWCR =	(1<<TWEN)|								// Enable TWI-interface and release TWI pins.
            (0<<TWIE)|(0<<TWINT)|					// Disable Interrupt.
            (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|		// No Signal requests.
            (0<<TWWC);								//
}


/****************************************************************************
Call this function to send a prepared message. The first byte must contain the slave address and the
read/write bit. Consecutive bytes contain the data to be sent, or empty locations for data to be read
from the slave. Also include how many bytes that should be sent/read including the address byte.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWIStartTransceiverWithData( uint8_t *msg, uint8_t msgSize )
{
    uint8_t temp;
    
    TWI_msgSize = msgSize;							// Number of data to transmit.
    TWI_buf[0]  = msg[0];							// Store slave address with R/W setting.
    if (!BitIsSet(msg[0],TWI_READ_BIT))			// If it is a write operation, then also copy data.
    {
        for ( temp = 1; temp < msgSize; temp++ )
        TWI_buf[ temp ] = msg[ temp ];
    }
    
    TWIStartTransceiver();
}

void TWIStartTransceiver( void )
{
    static uint8_t TWI_bufPtr;
    BOOL twi_finished = FALSE;

    TWI_lastTransOK = FALSE;
    twiState = TWI_NO_STATE;					    // Reset TWI_state.
    TWCR =	(1<<TWEN)|							    // TWI Interface enabled
    (0<<TWIE)|(1<<TWINT)|				            // Disable TWI Interrupt and clear the flag.
    (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|		        // Initiate a START condition.
    (0<<TWWC);								        //
    
    while(!twi_finished)
    {
        // Wait for TWINT flag
        while(!bit_is_set(TWCR, TWINT));

        switch (TWSR)
        {
            case TWI_START:							    // START has been transmitted
            case TWI_REP_START:						    // Repeated START has been transmitted
            TWI_bufPtr = 0;							    // Set buffer pointer to the TWI Address location
            case TWI_MTX_ADR_ACK:					    // SLA+W has been transmitted and ACK received
            case TWI_MTX_DATA_ACK:					    // Data byte has been transmitted and ACK received
            if (TWI_bufPtr < TWI_msgSize)
            {
                TWDR = TWI_buf[TWI_bufPtr++];
                TWCR = (1<<TWEN)|					    // TWI Interface enabled
                //(1<<TWIE)|(1<<TWINT)|				    // Enable TWI Interrupt and clear the flag to send byte
                (0<<TWIE)|(1<<TWINT)|				    // Clear the flag to send byte
                (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	    //
                (0<<TWWC);							    //
            }else									    // Send STOP after last byte
            {
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
            if (TWI_bufPtr < (TWI_msgSize-1) )		    // Detect the last byte to NACK it.
            {
                TWCR = (1<<TWEN)|						// TWI Interface enabled
                //(1<<TWIE)|(1<<TWINT)|				    // Enable TWI Interrupt and clear the flag to read next byte
                (0<<TWIE)|(1<<TWINT)|				    // Clear the flag to read next byte
                (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	    // Send ACK after reception
                (0<<TWWC);							    //
            }else									    // Send NACK after next reception
            {
                TWCR = (1<<TWEN)|					    // TWI Interface enabled
                //(1<<TWIE)|(1<<TWINT)|				    // Enable TWI Interrupt and clear the flag to read next byte
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
            case TWI_ARB_LOST:						    // Arbitration lost
            TWCR = (1<<TWEN)|						    // TWI Interface enabled
            //(1<<TWIE)|(1<<TWINT)|					    // Enable TWI Interrupt and clear the flag
            (0<<TWIE)|(1<<TWINT)|					    // Clear the flag
            (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|		    // Initiate a (RE)START condition.
            (0<<TWWC);								    //
            break;
            //case TWI_MTX_ADR_NACK:					    // SLA+W has been transmitted and NACK received
            //case TWI_MRX_ADR_NACK:					    // SLA+R has been transmitted and NACK received
            //case TWI_MTX_DATA_NACK:					    // Data byte has been transmitted and NACK received
            ////    case TWI_NO_STATE					    // No relevant state information available; TWINT = �0�
            //case TWI_BUS_ERROR:						    // Bus error due to an illegal START or STOP condition
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


/****************************************************************************
Call this function to read out the requested data from the TWI transceiver buffer. I.e. first call
TWI_Start_Transceiver to send a request for data to the slave. Then Run this function to collect the
data when they have arrived. Include a pointer to where to place the data and the number of bytes
requested (including the address field) in the function call. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation, before reading out the data and returning.
If there was an error in the previous transmission the function will return the TWI error code.
****************************************************************************/
uint8_t TWIGetDataFromTransceiver( uint8_t *msg, uint8_t msgSize )
{
    uint8_t i;

    
    if( TWI_lastTransOK )					// Last transmission competed successfully.
    {
        for ( i=0; i<msgSize; i++ )					// Copy data from Transceiver buffer.
        {
            msg[ i ] = TWI_buf[ i ];
        }
    }
    return( TWI_lastTransOK );
}

