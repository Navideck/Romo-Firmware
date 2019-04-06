/*
* pwm.h
*
* Created: 8/13/2012 8:15:43 PM
*  Author: Aaron Solochek
*/

#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>


typedef enum {
    PWM0,		// Right motor
    PWM1,		// Left motor
    PWM2		// Tilt motor
} PWM_CHANNEL;


void PWMInit(void);
void SetPWM(PWM_CHANNEL chan, int16_t val);

#define SetRightPWM(pwm)	SetPWM(PWM0, -pwm)
#define SetLeftPWM(pwm)		SetPWM(PWM1, pwm)
#define SetTiltPWM(pwm)		SetPWM(PWM2, -pwm)

#define FORWARD(pwm)		SetRightPWM(pwm); SetLeftPWM(pwm)
#define STOP()				SetRightPWM(0); SetLeftPWM(0)
#define TURN_LEFT(pwm)		SetLeftPWM(-pwm); SetRightPWM(pwm)
#define TURN_RIGHT(pwm)		SetRightPWM(-pwm); SetLeftPWM(pwm)
#define BACKWARD(pwm)		SetLeftPWM(-pwm); SetRightPWM(-pwm)
#define TILT_FORWARD(pwm)	SetTiltPWM(pwm)
#define TILT_BACKWARD(pwm)	SetTiltPWM(-pwm)
#define TILT_STOP()			SetTiltPWM(0)
#define STOP_ALL()          SetRightPWM(0); SetLeftPWM(0); SetTiltPWM(0)



#endif /* PWM_H_ */