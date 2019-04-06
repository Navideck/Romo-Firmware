/*
* current_control.c
*
* Created: 8/13/2012 8:23:33 PM
*  Author: Aaron Solochek
*/

#include "main.h"
#include "current_control.h"


int16_t leftCurrent, rightCurrent;
int16_t currentCommandADCLeft, currentCommandRefLeft;
int16_t currentCommandADCRight, currentCommandRefRight;
int16_t currentRefLeft, currentErrorLeft;
int16_t currentRefRight, currentErrorRight;
int16_t currentControlKi, currentControlKp;
int16_t currentControlKpScale, currentControlKiScale;
int16_t currentDBPosLimit, currentDBNegLimit;
int16_t currentSlewLeft, currentSlewRight, currentSlewRate;
int32_t currentControlPTermLeft, currentControlITermLeft;
int32_t currentControlPTermRight, currentControlITermRight;
int32_t currentControlIntegratorLeft, rawCurrentControlOutLeft, currentControlOutLeft;
int32_t currentControlIntegratorRight, rawCurrentControlOutRight, currentControlOutRight;




void CurrentControlInit()
{
    currentControlIntegratorLeft = 0;
    currentControlIntegratorRight = 0;
    currentControlPTermLeft = 0;
    currentControlPTermRight = 0;
    currentControlITermLeft = 0;
    currentControlITermRight = 0;
    rawCurrentControlOutLeft = 0;
    rawCurrentControlOutRight = 0;
    currentControlOutLeft = 0;
    currentControlOutRight = 0;
    currentControlKi = 100;
    currentControlKp = 40;
    currentRefLeft = 0;
    currentRefRight = 0;
    currentErrorLeft = 0;
    currentErrorRight = 0;
    currentCommandADCLeft = 0;
    currentCommandADCRight = 0;
    currentCommandRefLeft = 0;
    currentCommandRefRight = 0;
    currentSlewLeft = 0;
    currentSlewRight = 0;
    currentSlewRate = 3;
    currentDBPosLimit = 25;
    currentDBNegLimit = -25;
}


void CurrentControlStep()
{
    int16_t rightCurrentADC = devInfo.analogValues[ADC_M1CURRENT];
    int16_t leftCurrentADC = devInfo.analogValues[ADC_M2CURRENT];


    // Current reading Deadband
    if(rightCurrentADC > currentDBPosLimit)
    {
        rightCurrentADC -= currentDBPosLimit;
    }
    else if(rightCurrentADC < currentDBNegLimit)
    {
        rightCurrentADC -= currentDBNegLimit;
    }
    else
    {
        rightCurrentADC = 0;
    }

    if(leftCurrentADC > currentDBPosLimit)
    {
        leftCurrentADC -= currentDBPosLimit;
    }
    else if(leftCurrentADC < currentDBNegLimit)
    {
        leftCurrentADC -= currentDBNegLimit;
    }
    else
    {
        leftCurrentADC = 0;
    }



    //Slew to desired current
    if(currentSlewLeft > leftCurrentADC)
    {
        currentSlewLeft -= currentSlewRate;
    }
    else if(currentSlewLeft < leftCurrentADC)
    {
        currentSlewLeft += currentSlewRate;
    }


    //PI Controller
    //calculate current error
    currentErrorLeft = currentSlewLeft - currentRefLeft;
    currentErrorRight = currentSlewRight - currentRefRight;

    if(currentCommandRefLeft == 0)
    {
        currentControlIntegratorLeft = 0;
    }

    if(currentCommandRefRight == 0)
    {
        currentControlIntegratorRight = 0;
    }

    //integration
    currentControlIntegratorLeft += ((uint32_t)currentErrorLeft)*currentControlKi;
    currentControlIntegratorRight += ((uint32_t)currentErrorRight)*currentControlKi;

    //TODO: Check and control range of integrator

    currentControlPTermLeft = ((int32_t)currentErrorLeft)*currentControlKp;
    currentControlPTermRight = ((int32_t)currentErrorRight)*currentControlKp;

    rawCurrentControlOutLeft = (currentControlPTermLeft >> 3) + (currentControlITermLeft >> 8);
    rawCurrentControlOutLeft = (currentControlPTermRight >> 3) + (currentControlITermRight >> 8);

    

    //TODO: Check and control output range

    currentControlOutLeft = rawCurrentControlOutLeft >> 24;
    currentControlOutRight = rawCurrentControlOutRight >> 24;
    
}
