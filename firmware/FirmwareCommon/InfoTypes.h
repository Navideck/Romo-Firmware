//
//  InfoTypes.h
//  RMCore
//
//  Created by Aaron Solochek on 2013-04-08.
//  Copyright (c) 2013 Romotive. All rights reserved.
//

#ifndef INFOTYPES_H
#define INFOTYPES_H

typedef enum RMInfoType_t {
    RMInfoTypeNull,
    RMInfoTypeMotorCurrent, // <motor number>
    RMInfoTypeMotorVelocity, // <motor number>
    RMInfoTypeMotorPosition, // <motor number>
    RMInfoTypeMotorTorque, // <motor number>
    RMInfoTypeMotorTemp, // <motor number>
    RMInfoTypeMotorPWM, // <motor number>
    RMInfoTypeAllAnalog = 0x10, // <no arguments>
    RMInfoTypeChargingState,
    RMInfoTypeChargingCurrent,
} RMInfoType;



#endif
