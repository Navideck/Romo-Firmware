/*
* mfi.c
*
* Created: 9/19/2012 11:02:51 AM
*  Author: Dan Kane
*/

#if 0

#include "romo3.h"
#include "mfi.h"
#include "commands.h"
#include "ringbuffer.h"
#include "uart.h"

volatile struct mfiStatus_t mfiStatus;


void MFIInit()
{
    mfiStatus.transactionIDiPod = 0;
    mfiStatus.transactionIDRomo = 1;
    mfiStatus.sessionIDiPod = 0;
    mfiStatus.flags.all = 0;
    mfiStatus.communication.all = 0;
        
    // Initialize authentication co-processor
    MFIAuthInit();

    // UART Communication Startup
    _delay_ms(80);					// Wait 80ms
    UART1_TX_WHEN_READY(SYNC_BYTE);  // Send sync byte
    _delay_ms(20);					// Wait 20ms
}

void MFISendCommand(uint8_t commandID, uint8_t *data, uint8_t dataLen, volatile uint16_t *transID)
{
    int8_t checksum = (int8_t)(dataLen+4) + (int8_t)IPOD_LINGO_GENERAL + (int8_t)commandID + (int8_t)((*transID)>>8) + (int8_t)(*transID);
    
    UART1_TX_WHEN_READY(SYNC_BYTE);             // Sync Byte
    UART1_TX_WHEN_READY(START_BYTE);            // Start Byte
    UART1_TX_WHEN_READY(dataLen+4);             // Length byte = LingoID(1) + CommandID(1) + TransID(2) + Data(n) = n+4
    UART1_TX_WHEN_READY(IPOD_LINGO_GENERAL);    // LingoID
    UART1_TX_WHEN_READY(commandID);             // CommandID (StartIDPS)
//    UART1_TX_WHEN_READY((*transID)>>8);	        // TransID high byte
    UART1_TX_WHEN_READY(0x0);                   // TransID high byte will always be zero for this situation
    UART1_TX_WHEN_READY(*transID);		        // TransID low byte
    for (int i=0; i<dataLen; i++)
    {
        UART1_TX_WHEN_READY(data[i]);	        // Data
        checksum += data[i];
    }

    checksum ^= 0xFF;
    checksum += 0x01;
    UART1_TX_WHEN_READY(checksum);		        // Checksum
        
    if (transID == &mfiStatus.transactionIDRomo)
    {
        (*transID)++;						// Increment transaction ID for next use
    }        
}

BOOL MFIReceiveCommand(uint8_t *command, uint8_t *commandData, uint8_t *commandDataLen)
{
    // Check for incoming bytes and store them
    uint8_t theByte;
    uint8_t length;
    uint16_t transID;
    int8_t checksum;
    int8_t checksumShouldBe = 0;
    
    while (1)
    {
        while (!UART1_RX_COMPLETE);     // Wait for byte
        theByte = UDR1;
        
        if (theByte == START_BYTE)      // Got a packet!
        {
            
            while (!UART1_RX_COMPLETE);
            length = UDR1;
            
            while (!UART1_RX_COMPLETE);
            theByte = UDR1;
            
            while (!UART1_RX_COMPLETE);
            *command = UDR1;
            
            while (!UART1_RX_COMPLETE);
            transID = (UDR1<<8);
            while (!UART1_RX_COMPLETE);
            transID += UDR1;
            
            for (uint8_t i=0; i<(length-4); i++)
            {
                while (!UART1_RX_COMPLETE);
                commandData[i] = UDR1;
                checksumShouldBe += (int8_t)commandData[i];
            }
            
            while (!UART1_RX_COMPLETE);
            checksum = UDR1;
            
            // Finish checksum
            checksumShouldBe += (int8_t)length + (int8_t)IPOD_LINGO_GENERAL + (int8_t)(*command) + (int8_t)(transID>>8) + (int8_t)transID;
            checksumShouldBe ^= 0xFF;
            checksumShouldBe += 0x01;
            
            // Compare the checksum to what it should be
            if ((checksum == checksumShouldBe))
            {
                *commandDataLen = length-4;
                mfiStatus.transactionIDiPod = transID;
                return TRUE;
            }
            else
                return FALSE;
        }
    }
}


#endif //0