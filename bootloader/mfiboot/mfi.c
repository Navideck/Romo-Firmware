/*
* mfi.c
*
* Created: 8/16/2013 11:14:19 AM
*  Author: aarons
*/


#include <inttypes.h>
#include "mfi.h"
#include "util.h"
#include "mfiboot.h"
#include "ringbuffer.h"
#include "SerialProtocol.h"
#include "uart.h"
#include <util/delay.h>

register uint8_t mfiCommand asm("r9");
register uint8_t mfiCommandDataLen asm("r10");
register uint8_t sessionID asm("r11");

uint8_t pCommandData[CMD_MAX_PAYLOAD+9];
uint16_t transIDiPod;
uint16_t transIDRomo;


// Send the mfi command out over the serial port
void MFITransmitCommand(uint8_t commandID, uint8_t dataLen, uint16_t transID)
{
    int8_t checksum = 0;

    putch(SYNC_BYTE);			// Sync Byte
    putch(START_BYTE);			// Start Byte
    putch(dataLen+4);			// Length byte = LingoID(1) + CommandID(1) + TransID(2) + Data(n) = n+4
    putch(IPOD_LINGO_GENERAL);	// LingoID
    putch(commandID);			// CommandID (StartIDPS)
    putch(transID>>8);          // TransID high byte
    putch(transID);	            // TransID low byte
    
    for(int i=0; i<dataLen; i++) {
        putch(data[i]);			// Data
        checksum += (int8_t)data[i];
    }
    checksum += (int8_t)(dataLen+4) + (int8_t)IPOD_LINGO_GENERAL + (int8_t)commandID + (int8_t)(HIGH_BYTE(transID)) + (int8_t)(LOW_BYTE(transID));
    checksum ^= 0xFF;
    checksum += 0x01;

    putch(checksum);            // Checksum
}


// Send an MFI acknowledgement packet for the given commandID with the given result
void MFISendAck(uint8_t cmd, uint8_t result)
{
    data[0] = result;
    data[1] = cmd;
    MFITransmitCommand(IPOD_AccessoryACK, 2, transIDiPod);
}


void MFISendCommand(uint8_t cmd, uint8_t len)
{
    MFITransmitCommand(cmd, len, transIDRomo++);
    waitingForAck = TRUE;
}


void MFISendAuthData(uint8_t cmd, uint8_t len)
{
    MFITransmitCommand(cmd, len, transIDiPod);
}


// See if the input buffer contains a valid MFI packet.
// If so, populate mfiCommand, mfiCommandDataLen, pCommandData, and transIDiPod
//
// Packet format and lengths:
// Minimum packet size is StartByte(1), LengthByte(1), LingoID(1), CommandID(1), TransID(2), No Data(0), Checksum(1) = 7 bytes
// Length = LingoID(1) + CommandID(1) + TransID(2) + Data(n) = n + 4 bytes
// Length + StartByte(1) + LengthByte(1) + Checksum(1) = length + 3 = all bytes in packet
void MFIGetCommand(void)
{
    newCommand = FALSE;
    
    // Start processing at the first start byte we find
    while(BytesUsedInBuf() && (PeekByte(&inBuf, 0) != START_BYTE)) {
        #ifdef USE_MUTEX
        if(MutexTryLockInBuf()) {
            GetByteInBuf();
            MutexUnlockInBuf();
        }
        else {
            _delay_us(10);
        }
        #else
        GetByteInBuf();
        #endif //USE_MUTEX
    }
    uint8_t length = PeekByte(&inBuf, 1); // Length
    
    // Make sure we have all the bytes in the packet, and the length is sane
    if((BytesUsedInBuf() >= length+3) && (length <= CMD_MAX_PAYLOAD)) {
        int8_t checksum = -1;
        int8_t checksumShouldBe = 0;

        mfiCommand = PeekByte(&inBuf, 3); // CommandID
        uint8_t transID_H = PeekByte(&inBuf, 4); // Transaction ID high byte
        uint8_t transID_L = PeekByte(&inBuf, 5); // Transaction ID low byte
        
        // Process data
        for(inIndex=6; inIndex<length+2; inIndex++) {
            pCommandData[inIndex-6] = PeekByte(&inBuf, inIndex);
            checksumShouldBe += (int8_t)pCommandData[inIndex-6];
        }
        checksum = PeekByte(&inBuf, length+2);

        // Finish checksum
        checksumShouldBe += (int8_t)length + (int8_t)IPOD_LINGO_GENERAL + (int8_t)mfiCommand + (int8_t)transID_H + (int8_t)transID_L;
        checksumShouldBe ^= 0xFF;
        checksumShouldBe += 0x01;
        
        // Compare the checksum to what it should be
        if(checksum == checksumShouldBe) {
            // Pull all these bytes out of the input buffer
            #ifdef USE_MUTEX
            MutexLockInBuf();
            #endif //USE_MUTEX
            
            DropBytesInBuf(length+3);
            
            #ifdef USE_MUTEX
            MutexUnlockInBuf();
            #endif //USE_MUTEX
            
            mfiCommandDataLen = length-4;
            transIDiPod = (uint16_t)((transID_H<<8) | transID_L);
            newCommand = TRUE;
        }
        else {
            // Pull off the start byte since this wasn't a valid packet

            #ifdef USE_MUTEX
            MutexLockInBuf();
            #endif //USE_MUTEX
            
            GetByteInBuf();

            #ifdef USE_MUTEX
            MutexUnlockInBuf();
            #endif //USE_MUTEX
        }
    }
}


void AddFIDTokenWithByte(uint8_t subType, uint8_t b, char *buf, uint8_t length)
{
    data[0]++;
    data[myIndex++] = length;
    data[myIndex++] = FID_TYPE_0;
    data[myIndex++] = subType;
    data[myIndex++] = b;
    memcpy(data+myIndex, buf, length-3);
    myIndex += length-3;
}


void AddFIDToken(uint8_t subType, char *buf, uint8_t length)
{
    AddFIDTokenWithByte(subType, buf[0], buf+1, length);
}


void AddFIDInfoToken(uint8_t infoType, char *buf, uint8_t length)
{
    AddFIDTokenWithByte(FID_ACCESSORY_INFO_TOKEN, infoType, buf, length);
}
