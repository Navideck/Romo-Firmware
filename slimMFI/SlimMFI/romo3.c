/*
 * romo3.c
 *
 * Created: 10/4/2012 1:42:43 PM
 *  Author: Dan Kane
 */ 


#include "romo3.h"
#include "util.h"
//#include "mfi_config.h"
//#include "ringbuffer.h"
#include "uart.h"
#include "twi.h"
//#include "mfi_ident.h"
#include "mfi_auth.h"
//#include "mfi.h"
#include <util/setbaud.h>



int main(void)
{
    LEDS_ENABLE;
    CPRESET_ENABLE;

 CPInit();								// Initialize the co-processor (sets I2C baud)
 
 // Run CP self test
 uint8_t selfTestData = 0x01;
 uint8_t protocolMajorVersion = 0;
 uint8_t protocolMinorVersion = 0;
 CPWrite(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);	// Send 1 to CP self test register to initiate
 CPRead(AUTH_SELFTEST_CONTROL_STATUS, &selfTestData, 1);		// Read results (bits 7-4 hold the result)
 
 if (bit_is_clear(selfTestData, CP_SELFTEST_X509_BIT) ||     // Bit 7 = x.509 certificate: 0=not found, 1=found
 bit_is_clear(selfTestData, CP_SELFTEST_PK_BIT))				// Bit 6 = private key: 0=not found, 1=found
 {
     return FALSE;
 }
 
 // CP self test passed!
 
 // Get Authentication Version from co-processor
 CPRead(AUTH_PROTOCOL_MAJOR_VERSION, &protocolMajorVersion, 1);
 CPRead(AUTH_PROTOCOL_MINOR_VERSION, &protocolMinorVersion, 1);



#if 0
    UART1Init();
    MFIInit();
    if (!MFIIdentify() || !MFIAuthenticate())
    {
        while (1)
        {
            LEDS_ON;
            _delay_ms(500);
            LEDS_OFF;
            _delay_ms(500);
            
        }
    }
#endif //0    
    
    LEDS_ON;
    
    while(1)
    {
         //if (UART1_RX_COMPLETE)
         //{
             //PutByte(inBuf, UDR1);
         //}
        
//        MFITasks();
//        MFIIdentTasks();
//        MFIAuthTasks();
    }
}