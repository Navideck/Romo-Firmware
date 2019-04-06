/*
* ringbuffer.c
*
* Created: 10/9/2012 5:01:10 PM
*  Author: Aaron Solochek
*/


#include "ringbuffer.h"
#include "mfiboot.h"


volatile Ringbuf inBuf;

#ifndef USE_INLINE_RINGBUFFER
void PutByteInBuf(uint8_t b)
{
    if(inBufUsed != RING_BUFFER_SIZE-1) {
        inBuf.data[AdvancePtr(&(inBuf.head))] = b;
        inBufUsed++;
    }
}


void PutByteOutBuf(uint8_t b)
{
    if(outBufUsed != RING_BUFFER_SIZE-1) {
        outBuf.data[AdvancePtr(&(outBuf.head))] = b;
        outBufUsed++;
    }
}


uint8_t GetByteInBuf()
{
    if(inBufUsed) {
        inBufUsed--;
        return inBuf.data[AdvancePtr(&(inBuf.tail))];
    }
    return 0;
}


uint8_t GetByteOutBuf()
{
    if(outBufUsed) {
        outBufUsed--;
        return outBuf.data[AdvancePtr(&(outBuf.tail))];
    }
    return 0;
}

uint8_t PeekByte(volatile Ringbuf *rbuf, uint8_t offset)
{
    uint16_t location = rbuf->tail + offset;
    if (location >= RING_BUFFER_SIZE) {
        location -= RING_BUFFER_SIZE;
    }
    return rbuf->data[location];
}
#endif


// read count bytes from the inBuf and store them dest, removing them from inBuf
// returns the number of bytes read
int GetBytesInBuf(uint8_t *dest, uint8_t count)
{
    int i;

    for(i = 0; i <= count; i++) {
        if(BytesUsedInBuf()) {
            dest[i] = GetByteInBuf();
        }
        else {
            break;
        }
    }
    return i;
}


// read count bytes from the outBuf and store them dest, removing them from inBuf
// returns the number of bytes read
int GetBytesOutBuf(uint8_t *dest, uint8_t count)
{
    int i;
    
    for(i = 0; i <= count; i++) {
        if(BytesUsedOutBuf()) {
            dest[i] = GetByteOutBuf();
        }
        else {
            break;
        }
    }
    return i;
}


// put count bytes from src into the inBuf
// returns number of bytes written
int PutBytesInBuf(uint8_t *src, uint8_t count)
{
    int i;
    
    for(i = 0; i <= count; i++) {
        if(BytesFreeInBuf()) {
            PutByteInBuf(src[i]);
        }
        else {
            break;
        }
    }
    return i;
}


// put count bytes from src into the outBuf
// returns number of bytes written
int PutBytesOutBuf(uint8_t *src, uint8_t count)
{
    int i;
    
    for(i = 0; i <= count; i++) {
        if(BytesFreeInBuf()) {
            PutByteInBuf(src[i]);
        }
        else {
            break;
        }
    }
    return i;
}

