/*
* util.h
*
* Created: 8/13/2012 8:15:14 PM
*  Author: Aaron Solochek
*/


#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

#define	cbi(sfr, bit)			(_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)			(_SFR_BYTE(sfr) |= _BV(bit))
#define BitIsSet(obj, bit)      (obj & _BV(bit))

#define HIGH_BYTE(w)                 ((w >> 8) & 0xFF)
#define LOW_BYTE(w)                  (w & 0xFF)

// Evaluates to the largest unsigned value that can fit in t
#define U_MAX(t)                (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
(0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

// Evaluates to the largest signed value that can fit in t
#define S_MAX(t)                (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
(0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

// Cause a compile error if e is not 0
//#define ERROR_IF_NON_ZERO(e) (sizeof(struct { int:-!!(e); }))
//#define ERROR_IF_NON_NULL(e, m) ((void *)sizeof(struct { int:-!!(e); }))

#define ERROR_IF_ZERO(e, m)     (sizeof(struct {int m:e; }))
#define ERROR_IF_NULL(e, m)     ((void *)sizeof(struct { int m:e; }))

// These macros will cause a compile error if the value of e will not fit
// in the storage type of v
#define U_BOUNDS_CHECK(e,v)     (ERROR_IF_ZERO(!(e > U_MAX(v)), value_out_of_bounds) ? e : e)
#define S_BOUNDS_CHECK(e,v)     (ERROR_IF_ZERO(!(e > S_MAX(v)), value_out_of_bounds) ? e : e)

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;    /* Undefined size */


#define A_TO_I(x)					(x-48)
#define I_TO_A(x)					(x+48)



inline char NibbleToAscii(uint8_t b)
{
    if((b & 0x0F) < 10) {
      return (b & 0x0F) + 48;
    }
    return (b & 0x0f) + 55;
}


inline void Int8toAscii(uint8_t n, char *str)
{
    str[0] = NibbleToAscii(n >> 4);
    str[1] = NibbleToAscii(n);
}

inline void Int16toAscii(uint16_t n, char *str)
{
    str[0] = NibbleToAscii(n >> 12);
    str[1] = NibbleToAscii(n >> 8);
    str[2] = NibbleToAscii(n >> 4);
    str[3] = NibbleToAscii(n);
}


inline uint16_t Read16(uint8_t *buf, uint8_t offset)
{
    union {
        uint16_t x;
        struct {
            uint8_t a;
            uint8_t b;
        };
    } out;

    out.b = buf[offset];
    out.a = buf[offset+1];
    return out.x;
}


inline uint32_t Read32(uint8_t *buf, uint8_t offset)
{
    union {
        uint32_t x;
        struct {
            uint8_t a;
            uint8_t b;
            uint8_t c;
            uint8_t d;
        };
    } out;

    out.d = buf[offset];
    out.c = buf[offset+1];
    out.b = buf[offset+2];
    out.a = buf[offset+3];
    return out.x;
}

extern int abs(int __i);


// prototype to get rid of annoying warnings
void *memcpy(void *, const void *, unsigned int);
void *memset(void *, int, unsigned int);
int strncmp(const char *, const char *, unsigned int);

#endif /* UTIL_H_ */