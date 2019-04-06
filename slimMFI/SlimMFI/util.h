/*
* util.h
*
* Created: 8/13/2012 8:15:14 PM
*  Author: Aaron Solochek
*/


#ifndef UTIL_H_
#define UTIL_H_

//#include <stdint.h>

#define	cbi(sfr, bit)			(_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)			(_SFR_BYTE(sfr) |= _BV(bit))
#define BitIsSet(obj, bit)      (obj & _BV(bit))

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;    /* Undefined size */


#endif /* UTIL_H_ */