/*
* flags.h
*
* Created: 10/9/2012 4:48:20 PM
*  Author: Aaron Solochek
*/


#ifndef FLAGS_H_
#define FLAGS_H_

#include <inttypes.h>

typedef struct devFlags_t {
    uint8_t loopTrigger_f:1;
    uint8_t ledPulseIncreasing_f:1;
    uint8_t outBufLocked_f:1;
    uint8_t inBufLocked_f:1;
    uint8_t waitTriggered_f:1;
    uint8_t chargeLEDState_f:1;
    uint8_t chargeLEDStable_f:1;
    uint8_t reset_f:1;
} devFlags1;




#endif /* FLAGS_H_ */