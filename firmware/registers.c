/*
* registers.c
*
* Created: 8/15/2012 1:50:22 AM
*  Author: Aaron Solochek
*/

#include "main.h"


volatile struct GPIO_t* GPIO;
volatile struct TIMERSTATUS_t* TIMERSTATUS;
volatile struct EXTERNALINTERRUPTS_t* EXTERNALINTERRUPTS;
volatile struct EEPROM_t* EEPROM;
volatile union GTCCR_REG* TIMERCONTROL;
volatile struct TIMER0_t* TIMER0;
volatile struct SPI_t* SPI;
volatile union ACSR_REG* ANALOGCOMPARATOR;
volatile struct MCU_t* MCU;
volatile union SPMCSR_REG* SPM;
volatile union WDTCSR_REG* WATCHDOG;
volatile union CLKPR_REG* CLOCKPRESCALER;
volatile union PRR_REG* POWERREDUCTION;
volatile struct EXTERNALINTERRUPTSCONTROL_t* EXTERNALINTERRUPTSCONTROL;
volatile struct TIMERINTERRUPTMASK_t* TIMERINTERRUPTMASK;
volatile struct ANALOGIN_t* ANALOGIN;
volatile struct TIMER_t* TIMER1;
volatile struct TIMER_t* TIMER3;
volatile struct TIMER2_t* TIMER2;
volatile struct TWI_t* TWI;
volatile struct UART_t* UART0;
volatile struct UART_t* UART1;


void RegistersInit()
{
    GPIO = (volatile struct GPIO_t*) 0x20; //Start address of port registers
    TIMERSTATUS = (volatile struct TIMERSTATUS_t*) 0x35;
    EXTERNALINTERRUPTS = (volatile struct EXTERNALINTERRUPTS_t*) 0x3B;
    EEPROM = (volatile struct EEPROM_t*) 0x3F;
    TIMERCONTROL = (volatile union GTCCR_REG*) 0x43;
    TIMER0 = (volatile struct TIMER0_t*) 0x44;
    SPI = (volatile struct SPI_t*) 0x4C;
    ANALOGCOMPARATOR = (volatile union ACSR_REG*) 0x50;
    MCU = (volatile struct MCU_t*) 0x53;
    SPM = (volatile union SPMCSR_REG*) 0x57;
    WATCHDOG = (volatile union WDTCSR_REG*) 0x60;
    CLOCKPRESCALER = (volatile union CLKPR_REG*) 0x61;
    POWERREDUCTION = (volatile union PRR_REG*) 0x64;
    EXTERNALINTERRUPTSCONTROL = (volatile struct EXTERNALINTERRUPTSCONTROL_t*) 0x68;
    TIMERINTERRUPTMASK = (volatile struct TIMERINTERRUPTMASK_t*) 0x6E;
    ANALOGIN = (volatile struct ANALOGIN_t*) 0x78;
    TIMER1 = (volatile struct TIMER_t*) 0x80;
    TIMER3 = (volatile struct TIMER_t*) 0x90;
    TIMER2 = (volatile struct TIMER2_t*) 0xB0;
    TWI = (volatile struct TWI_t*) 0xB8;
    UART0 = (volatile struct UART_t*) 0xC0;
    UART1 = (volatile struct UART_t*) 0xC8;
    
    RegistersReset();
}


/*
 * Put the registers in a good state before a soft reset for the bootloader
 */
void RegistersReset()
{
    register uint8_t r3 asm("r3");
    register uint8_t r4 asm("r4");
    register uint8_t r5 asm("r5");
    register uint8_t r6 asm("r6");
    register uint8_t r7 asm("r7");
    register uint8_t r8 asm("r8");
    register uint8_t r9 asm("r9");
    register uint8_t r10 asm("r10");
    register uint8_t r11 asm("r11");

    r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = 0;
    //GPIOR0 = GPIOR1 = GPIOR2 = r3;
    DIDR1 = r3;
    //DDRB = 0x19;
    //DDRD = 0xFA;
}


//DEFREG(PINA, 0x20);
//DEFREG(PORTA, 0x21);


//volatile union PORTA_REG * PORTA_U = 0x22;

//volatile union EECR_REG * EECR_U = 0x3F;