/*
* ringbuffer.h
*
* Created: 8/24/2012 3:49:14 PM
*  Author: Aaron Solochek
*
* This ringbuffer is very application specific.  There are two buffers defined,
* inBuf and outBuf, and specific functions of accessing each.  This reduces
* execution time at the expense of code size.
*
*/

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include "flags.h"
#include "SerialProtocol.h"
#include <avr/interrupt.h>

#define RING_BUFFER_SIZE			CMD_MAX_PAYLOAD+9


typedef struct Ringbuf_t {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t data[RING_BUFFER_SIZE];
} Ringbuf;


// Use GPIO registers to store variables that are heavily accessed for efficiency
#define inBufMutex ((devFlags1*)&GPIOR0)->inBufLocked_f
#define outBufMutex ((devFlags1*)&GPIOR0)->outBufLocked_f
#define inBufUsed GPIOR1
#define outBufUsed GPIOR2


extern volatile Ringbuf outBuf;
extern volatile Ringbuf inBuf;


// discard the contents of the input buffer
static inline void FlushInBuf()
{
    inBuf.head = inBuf.tail = 0;
    inBufUsed = 0;
}


// discard the contents of the output buffer
static inline void FlushOutBuf()
{
    outBuf.head = outBuf.tail = 0;
    outBufUsed = 0;
}


// return the next value for the index provided
static inline int AdvancePtr(volatile uint8_t *ptr)
{
    if(*ptr == RING_BUFFER_SIZE - 1) {
        *ptr = 0;
        return RING_BUFFER_SIZE - 1;
    }
    return (*ptr)++;
}


// return the value of a byte at an offset in the given buffer.
// since this function does not modify the buffers, it can
// operate on a generic buffer
static inline uint8_t PeekByte(volatile Ringbuf *rbuf, uint8_t offset)
{
    uint16_t location = rbuf->tail + offset;
    if (location >= RING_BUFFER_SIZE) {
        location -= RING_BUFFER_SIZE;
    }
    return rbuf->data[location];
}


// Drop count bytes off the input buffer
static inline void DropBytesInBuf(uint8_t count)
{
    uint16_t location = inBuf.tail + count;
    if (location >= RING_BUFFER_SIZE) {
        location -= RING_BUFFER_SIZE;
    }
    inBuf.tail = location;
    inBufUsed -= count;
}


// insert the byte at the end of the buffer
// returns 0 on success, -1 if the buffer is full
static inline int PutByteInBuf(uint8_t b)
{
    if(inBufUsed == RING_BUFFER_SIZE-1) {
        return -1;
    } else {
        inBuf.data[AdvancePtr(&(inBuf.head))] = b;
        inBufUsed++;
    }
    return 0;
}


// insert the byte at the end of the buffer
// returns 0 on success, -1 if the buffer is full
static inline int PutByteOutBuf(uint8_t b)
{
    if(outBufUsed == RING_BUFFER_SIZE-1) {
        return -1;
    } else {
        outBuf.data[AdvancePtr(&(outBuf.head))] = b;
        outBufUsed++;
    }
    return 0;
}


// return the oldest byte in the buffer and remove it
// the caller must be careful not to call this if the
// buffer is empty
static inline uint8_t GetByteInBuf()
{
    inBufUsed--;
    return inBuf.data[AdvancePtr(&(inBuf.tail))];
}


// return the oldest byte in the buffer and remove it
// the caller must be careful not to call this if the
// buffer is empty
static inline uint8_t GetByteOutBuf()
{
    outBufUsed--;
    return outBuf.data[AdvancePtr(&(outBuf.tail))];
}


// return the number of bytes used in the inBuf
static inline int BytesUsedInBuf()
{
    return inBufUsed;
}


// return the number of bytes free in the inBuf
static inline int BytesFreeInBuf()
{
    return RING_BUFFER_SIZE - BytesUsedInBuf();
}


// return the number of bytes used in the outBuf
static inline int BytesUsedOutBuf()
{
    return outBufUsed;
}


// return the number of bytes free in the outBuf
static inline int BytesFreeOutBuf()
{
    return RING_BUFFER_SIZE - BytesUsedOutBuf();
}


// block until the outBuf is available, then lock it
// and return
inline void MutexLockOutBuf()
{
    while(outBufMutex);
    outBufMutex = 1;
}


// attempt to lock the outBuf
// return 1 on success, 0 if the outBuf is already locked
inline int MutexTryLockOutBuf()
{
    if(!outBufMutex) {
        outBufMutex = 1;
        return 1;
    } else {
        return 0; //mutex is already locked
    }
}


// unlock the outBuf
inline void MutexUnlockOutBuf()
{
    outBufMutex = 0;
}


// block until the inBuf is available, then lock it
// and return
inline void MutexLockInBuf()
{
    while(inBufMutex);
    inBufMutex = 1;
}


// attempt to lock the inBuf
// return 1 on success, 0 if the inBuf is already locked
inline int MutexTryLockInBuf()
{
    if(!inBufMutex) {
        inBufMutex = 1;
        return 1;
    } else {
        return 0; //mutex is already locked
    }
}


// unlock the inBuf
inline void MutexUnlockInBuf()
{
    inBufMutex = 0;
}


int GetBytesInBuf(uint8_t *dest, uint8_t count);
int GetBytesOutBuf(uint8_t *dest, uint8_t count);
int PutBytesInBuf(uint8_t *src, uint8_t count);
int PutBytesOutBuf(uint8_t *src, uint8_t count);



#endif /* RINGBUFFER_H_ */