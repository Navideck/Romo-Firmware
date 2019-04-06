/*
* ringbuffer.c
*
* Created: 10/9/2012 5:01:10 PM
*  Author: Aaron Solochek
*/


#include "main.h"
#include "ringbuffer.h"

volatile Ringbuf inBuf;
volatile Ringbuf outBuf;


// read count bytes from the inBuf and store them dest, removing them from inBuf
// returns the number of bytes read
int GetBytesInBuf(uint8_t *dest, uint8_t count)
{
    int i;

    for(i = 0; i <= count; i++) {
        if(BytesUsedInBuf()) {
            dest[i] = GetByteInBuf();
        } else {
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
        } else {
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
        } else {
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
        } else {
            break;
        }
    }
    return i;
}