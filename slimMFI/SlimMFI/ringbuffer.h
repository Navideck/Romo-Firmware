/*
* ringbuffer.h
*
* Created: 8/24/2012 3:49:14 PM
*  Author: Aaron Solochek
*/


#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#define RING_BUFFER_SIZE			IPOD_COMMAND_PAYLOAD_SIZE+9

#include "romo3.h"
#include "mfi_config.h"

typedef struct Ringbuf_t {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t data[RING_BUFFER_SIZE];
} Ringbuf;

//extern volatile Ringbuf *outBuf;
extern volatile Ringbuf inBuf;

register uint8_t inBufMutex asm("r2");
#define inBufUsed (uint8_t)GPIOR1



#endif /* RINGBUFFER_H_ */