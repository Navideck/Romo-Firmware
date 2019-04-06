/*
* registers.h
*
* Created: 8/13/2012 8:15:01 PM
*  Author: Aaron Solochek
*/


#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <inttypes.h>

#define OUTPUT 1
#define INPUT 0

#define PULLUP_ENABLED 1
#define PULLUP_DISABLED 0

//////////////////////////////////////////////////////////////////////////
// IOPORT Registers  0x20
//////////////////////////////////////////////////////////////////////////
union IOPORT_REG {
    uint8_t all;
    struct {
        uint8_t P0:1;
        uint8_t P1:1;
        uint8_t P2:1;
        uint8_t P3:1;
        uint8_t P4:1;
        uint8_t P5:1;
        uint8_t P6:1;
        uint8_t P7:1;
    };
};

struct IOPORT_t {
    union IOPORT_REG pin;
    union IOPORT_REG direction;
    union {
        union IOPORT_REG output;
        union IOPORT_REG pullup;
    };        
};

struct GPIO_t {
    volatile struct IOPORT_t portA;
    volatile struct IOPORT_t portB;
    volatile struct IOPORT_t portC;
    volatile struct IOPORT_t portD;
};

extern volatile struct GPIO_t* GPIO;


//////////////////////////////////////////////////////////////////////////
// Timer Status Registers  0x35
//////////////////////////////////////////////////////////////////////////
union TIMERSTATUS_REG {
    uint8_t all;
    struct {
        uint8_t OVERFLOW_FLAG:1;
        uint8_t COMPARE_MATCH_A_FLAG:1;
        uint8_t COMPARE_MATCH_B_FLAG:1;
        uint8_t RSVD:2;
        uint8_t INPUT_CAPTURE_FLAG:1;
    };
};

struct TIMERSTATUS_t {
    volatile union TIMERSTATUS_REG timer0;
    volatile union TIMERSTATUS_REG timer1;
    volatile union TIMERSTATUS_REG timer2;
    volatile union TIMERSTATUS_REG timer3;
};

extern volatile struct TIMERSTATUS_t* TIMERSTATUS;


//////////////////////////////////////////////////////////////////////////
// External Interrupt Registers  0x3B
//////////////////////////////////////////////////////////////////////////
union PCIFR_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT0_FLAG:1;
        uint8_t INTERRUPT1_FLAG:1;
        uint8_t INTERRUPT2_FLAG:1;
        uint8_t INTERRUPT3_FLAG:1;
        uint8_t RSVD:4;
    };
};

union EIFR_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT0_FLAG:1;
        uint8_t INTERRUPT1_FLAG:1;
        uint8_t INTERRUPT2_FLAG:1;
        uint8_t RSVD:5;
    };
};

union EIMSK_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT0:1;
        uint8_t INTERRUPT1:1;
        uint8_t INTERRUPT2:1;
        uint8_t RSVD:5;
    };
};

struct EXTERNALINTERRUPTS_t
{
    union PCIFR_REG pinChangeFlags;
    union EIFR_REG externalInterruptFlags;
    union EIMSK_REG externalInterruptMask;
};

extern volatile struct EXTERNALINTERRUPTS_t* EXTERNALINTERRUPTS;


//////////////////////////////////////////////////////////////////////////
// EEPROM Registers  0x3F
//////////////////////////////////////////////////////////////////////////
typedef enum EEPROMMode_t {
    EEPROM_ERASE_AND_WRITE,
    EEPROM_ERASE,
    EEPROM_WRITE,
    RSVD
} EEPROMModes;

union EECR_REG {
    uint8_t all;
    struct {
        uint8_t READ_ENABLE:1;
        uint8_t WRITE_ENABLE:1;
        uint8_t MASTER_PROG_ENABLE:1;
        uint8_t INTERRUPT_ENABLE:1;
        EEPROMModes PROG_MODE:2;
        uint8_t RSVD:2;
    };
};

struct EEPROM_t
{
    union EECR_REG control;
    uint8_t data;
    union {
        uint16_t address;
        struct {
            uint8_t addressLow;
            uint8_t addressHigh;
        };
    };
};

extern volatile struct EEPROM_t* EEPROM;


//////////////////////////////////////////////////////////////////////////
// General Timer Control Registers  0x43
//////////////////////////////////////////////////////////////////////////
union GTCCR_REG {
    uint8_t all;
    struct {
        uint8_t TIMER01_PRESCALER_RESET:1;
        uint8_t TIMER2_PRESCALER_RESET:1;
        uint8_t RSVD:5;
        uint8_t SYNCHRONIZATION_MODE:1;
    };
};

//extern volatile union GTTCR_REG* TIMERCONTROL;

//////////////////////////////////////////////////////////////////////////
// Timer0 Registers  0x44
//////////////////////////////////////////////////////////////////////////
typedef enum CompareOutputMode_t {
    OUTPUT_DISCONNECTED,
    TOGGLE_OUTPUT_ON_MATCH,
    CLEAR_OUTPUT_ON_MATCH,
    SET_OUTPUT_ON_MATCH
} CompareOutputModes;

typedef enum WaveformGenerationMode8Bit_t {
    WAVEGEN_NORMAL,
    WAVEGEN_PWM_PHASE_CORRECT,
    WAVEGEN_CTC,
    WAVEGEN_PWM_FAST
} WaveformGenerationModes8Bit;

typedef enum TimerClockRate_t {
    TIMER_CLOCK_OFF,
    TIMER_CLOCK_DIV_1,
    TIMER_CLOCK_DIV_8,
    TIMER_CLOCK_DIV_64,
    TIMER_CLOCK_DIV_256,
    TIMER_CLOCK_DIV_1024,
    TIMER_CLOCK_EXTERNAL_FALLING,
    TIMER_CLOCK_EXTERNAL_RISING
} TimerClockRates;


union TCCR0A_REG {
    uint8_t all;
    struct {
        WaveformGenerationModes8Bit WAVEFORM_GENERATION_MODE_L:2;
        uint8_t RSVD:2;
        CompareOutputModes COMPARE_MATCH_B_MODE:2;
        CompareOutputModes COMPARE_MATCH_A_MODE:2;
    };
};

union TCCR0B_REG {
    uint8_t all;
    struct {
        TimerClockRates CLOCK_SELECT:3;
        uint8_t WAVEFORM_GENERATION_MODE_H:1;
        uint8_t RSVD:2;
        uint8_t FORCE_OUTPUT_B:1;
        uint8_t FORCE_OUTPUT_A:1;
    };
};

struct TIMER0_t {
    union TCCR0A_REG registerA;
    union TCCR0B_REG registerB;
    uint8_t count;
    uint8_t outputCompareA;
    uint8_t outputCompareB;
};

extern volatile struct TIMER0_t* TIMER0;


//////////////////////////////////////////////////////////////////////////
// SPI Registers
//////////////////////////////////////////////////////////////////////////
typedef enum SPIClockRate_t {
    SPI_CLOCK_DIV_4,
    SPI_CLOCK_DIV_16,
    SPI_CLOCK_DIV_64,
    SPI_CLOCK_DIV_128
} SPIClockRates;

union SPCR_REG {
    uint8_t all;
    struct {
        SPIClockRates CLOCK_RATE:2;
        uint8_t CLOCK_PHASE:1; //0=sample on leading edge, 1=sample on trailing edge
        uint8_t CLOCK_POLARITY:1; //0=idle low, 1=idle high
        uint8_t MASTER_SLAVE_SELECT:1;
        uint8_t DATA_ORDER:1; //0=MSB first, 1=LSB first
        uint8_t SPI_ENABLE:1;
        uint8_t SPI_INTERRUPT_ENABLE:1;
    };
};

union SPSR_REG {
    uint8_t all;
    struct {
        uint8_t SPEED_DOUBLE:1;
        uint8_t RSVD:5;
        uint8_t WRITE_COLLISION:1;
        uint8_t INTERRUPT_FLAG:1;
    };
};

struct SPI_t {
    union SPCR_REG control;
    union SPSR_REG status;
};

extern volatile struct SPI_t* SPI;


//////////////////////////////////////////////////////////////////////////
// Analog Comparator Register  0x50
//////////////////////////////////////////////////////////////////////////
typedef enum ComparatorInterruptMode_t {
    INTERRUPT_ON_OUTPUT_TOGGLE,
    INTERRUPT_ON_FALLING_EDGE = 2,
    INTERRUPT_ON_RISING_EDGE
} ComparatorInterruptModes;

union ACSR_REG {
    uint8_t all;
    struct {
        ComparatorInterruptModes INTERRUPT_MODE_SELECT:2; //00=interrupt on toggle, 10=on falling edge, 11=on rising edge
        uint8_t INPUT_CAPTURE_ENABLE:1;
        uint8_t INTERRUPT_ENABLE:1;
        uint8_t INTERRUPT_FLAG:1;
        uint8_t COMPARATOR_OUTPUT:1;
        uint8_t BANDGAP_SELECT:1;
        uint8_t COMPARATOR_DISABLE:1;
    };
};


//////////////////////////////////////////////////////////////////////////
// MCU Registers  0x53
//////////////////////////////////////////////////////////////////////////
typedef enum SleepMode_t {
    MCU_IDLE,
    MCU_ADC_NOISE_REDUCTION,
    MCU_POWER_DOWN,
    MCU_POWER_SAVE,
    MCU_STANDBY = 6,
    MCU_EXTENDED_STANDBY
} SleepModes;

union SMCR_REG {
    uint8_t all;
    struct {
        uint8_t SLEEP_ENABLE:1;
        SleepModes MODE_SELECT:3;
        uint8_t RSVD:4;
    };
};

union MCUSR_REG {
    uint8_t all;
    struct {
        uint8_t POWER_ON_RESET_FLAG:1;
        uint8_t EXTERNAL_RESET_FLAG:1;
        uint8_t BROWNOUT_RESET_FLAG:1;
        uint8_t WATCHDOG_RESET_FLAG:1;
        uint8_t JTAG_RESET_FLAG:1;
    };
};

union MCUCR_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT_VECTOR_CHANGE_ENABLE:1;
        uint8_t INTERRUPT_VECTOR_SELECT:1;
        uint8_t RSVD:2;
        uint8_t power_something:1;
        uint8_t BOD_SLEEP_ENABLE:1;
        uint8_t BOD_SLEEP:1;
        uint8_t RSVD2:1;
    };
};

struct MCU_t {
    union SMCR_REG sleepModeControl;
    union MCUSR_REG status;
    union MCUCR_REG control;
};

extern volatile struct MCU_t* MCU;


//////////////////////////////////////////////////////////////////////////
// Store Program Memory Register  0x57
//////////////////////////////////////////////////////////////////////////
union SPMCSR_REG {
    uint8_t all;
    struct {
        uint8_t STORE_PROGRAM_MEMORY_ENABLE:1;
        uint8_t PAGE_ERASE:1;
        uint8_t PAGE_WRITE:1;
        uint8_t BOOT_LOCK_BIT_SET:1;
        uint8_t READ_WHILE_WRITE_ENABLE:1;
        uint8_t SIGNATURE_ROW_READ:1;
        uint8_t READ_WHILE_WRITE_BUSY:1;
        uint8_t SPM_INTERRUPT_ENABLE:1;
    };
};


//////////////////////////////////////////////////////////////////////////
// Watchdog Timer Control Register 0x60
//////////////////////////////////////////////////////////////////////////
typedef enum WatchdogClockRate_t {
    WDT_2K_CYCLES,
    WDT_4K_CYCLES,
    WDT_8K_CYCLES,
    WDT_16K_CYCLES,
    WDT_32K_CYCLES,
    WDT_64K_CYCLES,
    WDT_128K_CYCLES,
    WDT_256K_CYCLES
} WatchdogClockRates;

union WDTCSR_REG {
    uint8_t all;
    struct {
        WatchdogClockRates TIMER_PRESCALER:3;
        uint8_t SYSTEM_RESET_ENABLE:1;
        uint8_t CHANGE_ENABLE:1;
        uint8_t TIMER_PRESCALER_DOUBLE:1;
        uint8_t INTERRUPT_ENABLE:1;
        uint8_t INTERRUPT_FLAG:1;
    };
};



//////////////////////////////////////////////////////////////////////////
// Clock Prescalar Register
//////////////////////////////////////////////////////////////////////////
typedef enum ClockRate_t {
    CLOCK_DIV_1,
    CLOCK_DIV_2,
    CLOCK_DIV_4,
    CLOCK_DIV_8,
    CLOCK_DIV_16,
    CLOCK_DIV_32,
    CLOCK_DIV_64,
    CLOCK_DIV_128,
    CLOCK_DIV_256
} ClockRates;

union CLKPR_REG {
    uint8_t all;
    struct {
        ClockRates PRESCALER_SELECT:4;
        uint8_t RSVD:3;
        uint8_t CHANGE_ENABLE:1;
    };
};


//////////////////////////////////////////////////////////////////////////
// Power Reduction Registers  0x64
//////////////////////////////////////////////////////////////////////////
union PRR0_REG {
    uint8_t all;
    struct {
        uint8_t ADC_SLEEP:1;
        uint8_t UART0_SLEEP:1;
        uint8_t SPI_SLEEP:1;
        uint8_t TIM1_SLEEP:1;
        uint8_t UART1_SLEEP:1;
        uint8_t TIM0_SLEEP:1;
        uint8_t TIM2_SLEEP:1;
        uint8_t TWI_SLEEP:1;
    };
};

union PRR1_REG {
    uint8_t all;
    struct {
        uint8_t TIM3_SLEEP:1;
        uint8_t RSVD:7;
    };
};

union PRR_REG {
    uint16_t all;
    struct {
        uint16_t ADC_SLEEP:1;
        uint16_t UART0_SLEEP:1;
        uint16_t SPI_SLEEP:1;
        uint16_t TIM1_SLEEP:1;
        uint16_t UART1_SLEEP:1;
        uint16_t TIM0_SLEEP:1;
        uint16_t TIM2_SLEEP:1;
        uint16_t TWI_SLEEP:1;
        uint16_t TIM3_SLEEP:1;
        uint16_t RSVD:7;
    };
};


//////////////////////////////////////////////////////////////////////////
// External Interrupt Control Registers  0x68
//////////////////////////////////////////////////////////////////////////
union PCICR_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT_ENABLE0:1;
        uint8_t INTERRUPT_ENABLE1:1;
        uint8_t INTERRUPT_ENABLE2:1;
        uint8_t INTERRUPT_ENABLE3:1;
        uint8_t RSVD:4;
    };
};

union EICRA_REG {
    uint8_t all;
    struct {
        uint8_t SENSE_CONTROL0:2;
        uint8_t SENSE_CONTROL1:2;
        uint8_t SENSE_CONTROL2:2;
        uint8_t RSVD:2;
    };
};

union PCMSK0_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT0:1;
        uint8_t INTERRUPT1:1;
        uint8_t INTERRUPT2:1;
        uint8_t INTERRUPT3:1;
        uint8_t INTERRUPT4:1;
        uint8_t INTERRUPT5:1;
        uint8_t INTERRUPT6:1;
        uint8_t INTERRUPT7:1;
    };
};

union PCMSK1_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT8:1;
        uint8_t INTERRUPT9:1;
        uint8_t INTERRUPT10:1;
        uint8_t INTERRUPT11:1;
        uint8_t INTERRUPT12:1;
        uint8_t INTERRUPT13:1;
        uint8_t INTERRUPT14:1;
        uint8_t INTERRUPT15:1;
    };
};

union PCMSK2_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT16:1;
        uint8_t INTERRUPT17:1;
        uint8_t INTERRUPT18:1;
        uint8_t INTERRUPT19:1;
        uint8_t INTERRUPT20:1;
        uint8_t INTERRUPT21:1;
        uint8_t INTERRUPT22:1;
        uint8_t INTERRUPT23:1;
    };
};

union PCMSK3_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT24:1;
        uint8_t INTERRUPT25:1;
        uint8_t INTERRUPT26:1;
        uint8_t INTERRUPT27:1;
        uint8_t INTERRUPT28:1;
        uint8_t INTERRUPT29:1;
        uint8_t INTERRUPT30:1;
        uint8_t INTERRUPT31:1;
    };
};

struct EXTERNALINTERRUPTSCONTROL_t {
    union PCICR_REG pinChangeControl;
    union EICRA_REG externalInterruptControl;
    uint8_t RSVD;
    union PCMSK0_REG pinChangeMask0;
    union PCMSK1_REG pinChangeMask1;
    union PCMSK2_REG pinChangeMask2;
    uint32_t RSVD1;
    uint8_t RSVD2;
    union PCMSK3_REG pinChangeMask3;
};

extern volatile struct EXTERNALINTERRUPTSCONTROL_t* EXTERNALINTERRUPTSCONTROL;


//////////////////////////////////////////////////////////////////////////
// Timer Interrupt Mask Registers  0x6E
//////////////////////////////////////////////////////////////////////////
union TIMERINTERRUPTMASK_REG {
    uint8_t all;
    struct {
        uint8_t OVERFLOW_INTERRUPT_ENABLE	:1;
        uint8_t COMPARE_MATCH_A_INTERRUPT_ENABLE	:1;
        uint8_t COMPARE_MATCH_B_INTERRUPT_ENABLE	:1;
        uint8_t RSVD	:2;
        uint8_t INPUT_CAPTURE_INTERRUPT_ENABLE	:1;
    };
};

struct TIMERINTERRUPTMASK_t {
    union TIMERINTERRUPTMASK_REG timer0;
    union TIMERINTERRUPTMASK_REG timer1;
    union TIMERINTERRUPTMASK_REG timer2;
    union TIMERINTERRUPTMASK_REG timer3;
};

extern volatile struct TIMERINTERRUPTMASK_t* TIMERINTERRUPTMASK;


//////////////////////////////////////////////////////////////////////////
// ADC Control Registers
//////////////////////////////////////////////////////////////////////////
typedef enum ADCClockRate_t {
    ADC_CLOCK_DIV_2 = 1,
    ADC_CLOCK_DIV_4,
    ADC_CLOCK_DIV_8,
    ADC_CLOCK_DIV_16,
    ADC_CLOCK_DIV_32,
    ADC_CLOCK_DIV_64,
    ADC_CLOCK_DIV_128
} ADCClockRates;

typedef enum ADCVoltageRef_t {
    ADC_REF_AREF,
    ADC_REF_AVCC,
    ADC_REF_INTERNAL_1_1V,
    ADC_REF_INTERNAL_2_56V
} ADCVoltageRef;

typedef enum ADCTriggerSource_t {
    ADC_FREE_RUN,
    ADC_ANALOG_COMPARATOR,
    ADC_EXTERNAL_INTERRUPT,
    ADC_TIMER0_COMPARE_MATCH,
    ADC_TIMER0_OVERFLOW,
    ADC_TIMER1_COMPARE_MATCH,
    ADC_TIMER1_OVERFLOW,
    ADC_TIMER1_CAPTURE_EVENT
} ADCTriggerSources;

union ADCSR_REG {
    uint16_t all;
    struct {
        ADCClockRates PRESCALER:3;
        uint16_t INTERRUPT_ENABLE:1;
        uint16_t INTERRUPT_FLAG:1;
        uint16_t AUTO_TRIGGER_ENABLE:1;
        uint16_t START_CONVERSION:1;
        uint16_t ADC_ENABLE:1;
        ADCTriggerSources AUTO_TRIGGER_SOURCE:3;
        uint16_t unknown:3;
        uint16_t MULTIPLEXER_ENABLE:1;
        uint16_t RSVD:1;
    };
};

union ADMUX_REG {
    uint8_t all;
    struct {
        uint8_t ANALOG_CONFIG:5;
        uint8_t LEFT_ADJUST_RESULT:1;
        ADCVoltageRef VOLTAGE_REFERENCE_SELECT:2;
    };
};

union DIDR0_REG {
    uint8_t all;
    struct {
        uint8_t ADC0_DISABLE:1;
        uint8_t ADC1_DISABLE:1;
        uint8_t ADC2_DISABLE:1;
        uint8_t ADC3_DISABLE:1;
        uint8_t ADC4_DISABLE:1;
        uint8_t ADC5_DISABLE:1;
        uint8_t ADC6_DISABLE:1;
        uint8_t ADC7_DISABLE:1;
    };
};

union DIDR1_REG {
    uint8_t all;
    struct {
        uint8_t AIN0_DISABLE:1;
        uint8_t AIN1_DISABLE:1;
        uint8_t unknown:6;
    };
};

struct ANALOGIN_t {
    union {
        uint16_t data;
        struct {
            uint8_t dataLow;
            uint8_t dataHigh;
        };
    };
    union ADCSR_REG control;
    union ADMUX_REG mux;
    uint8_t RSVD;
    union DIDR0_REG digitalInputDisable0;
    union DIDR1_REG digitalInputDisable1;
};

extern volatile struct ANALOGIN_t* ANALOGIN;


//////////////////////////////////////////////////////////////////////////
// Timer 1,3 Registers  0x80, 0x90
//////////////////////////////////////////////////////////////////////////
typedef enum WaveformGenerationMode10Bit_t {
    WAVEGEN_NORMAL_10_BIT,
    WAVEGEN_PWM_PHASE_CORRECT_8_BIT,
    WAVEGEN_PWM_PHASE_CORRECT_9_BIT,
    WAVEGEN_PWM_PHASE_CORRECT_10_BIT,
    WAVEGEN_CTC_OCRA,
    WAVEGEN_PWM_FAST_8_BIT,
    WAVEGEN_PWM_FAST_9_BIT,
    WAVEGEN_PWM_FAST_10_BIT,
    WAVEGEN_PWM_PHASE_AND_FREQUENCY_CORRECT_ICR,
    WAVEGEN_PWM_PHASE_AND_FREQUENCY_CORRECT_OCRA,
    WAVEGEN_PWM_PHASE_CORRECT_ICR,
    WAVEGEN_PWM_PHASE_CORRECT_OCRA,
    WAVEGEN_CTC_ICR,
    WAVEGEN_PWM_FAST_ICR = 14,
    WAVEGEN_PWM_FAST_OCRA
} WaveformGenerationModes10Bit;

union TCCRA_REG {
    uint8_t all;
    struct {
        uint8_t WAVEFORM_GENERATION_MODE_L:2;
        uint8_t RSVD:2;
        CompareOutputModes COMPARE_MATCH_A_MODE:2;
        CompareOutputModes COMPARE_MATCH_B_MODE:2;
    };
};

union TCCRB_REG {
    uint8_t all;
    struct {
        uint8_t CLOCK_SELECT:3;
        uint8_t WAVEFORM_GENERATION_MODE_H:2;
        uint8_t RSVD:1;
        uint8_t INPUT_CAPTURE_EDGE_SELECT:1;
        uint8_t INPUT_CAPTURE_NOISE_CANCELER:1;
    };
};

union TCCRC_REG {
    uint8_t all;
    struct {
        uint8_t RSVD:6;
        uint8_t FORCE_OUTPUT_B:1;
        uint8_t FORCE_OUTPUT_A:1;
    };
};

struct TIMER_t {
    union TCCRA_REG registerA;
    union TCCRB_REG registerB;
    union TCCRC_REG registerC;
    uint8_t RSVD;
    union {
        uint16_t count;
        struct {
            uint8_t countLow;
            uint8_t countHigh;
        };
    };
    union {
        uint16_t inputCapture;
        struct {
            uint8_t inputCaptureLow;
            uint8_t inputCaptureHigh;
        };
    };
    union {
        uint16_t outputCompareA;
        struct {
            uint8_t outputCompareALow;
            uint8_t outputCompareAHigh;
        };
    };
    union {
        uint16_t outputCompareB;
        struct {
            uint8_t	outputCompareBLow;
            uint8_t outputCompareBHigh;
        };
    };
    
};

extern volatile struct TIMER_t* TIMER1;
extern volatile struct TIMER_t* TIMER3;


//////////////////////////////////////////////////////////////////////////
// Timer 2 Registers
//////////////////////////////////////////////////////////////////////////
union ASSR_REG {
    uint8_t all;
    struct {
        uint8_t CONTROL_REGISTER_B_BUSY	:1;
        uint8_t CONTROL_REGISTER_A_BUSY	:1;
        uint8_t OUTPUT_COMPARE_REGISTER_B_BUSY	:1;
        uint8_t OUTPUT_COMPARE_REGISTER_A_BUSY :1;
        uint8_t TIMER_REGISTER_BUSY	:1;
        uint8_t ASYNCHRONOUS		:1;
        uint8_t ENABLE_EXTERNAL_CLOCK_INPUT	:1;
        uint8_t RSVD:1;
    };
};

struct TIMER2_t {
    union TCCR0A_REG registerA;
    union TCCR0B_REG registerB;
    uint8_t count;
    uint8_t outputCompareA;
    uint8_t outputCompareB;
    uint8_t RSVD;
    union ASSR_REG asynchronousStatus;
};

extern volatile struct TIMER2_t* TIMER2;


//////////////////////////////////////////////////////////////////////////
// TWI Registers  0xB8
//////////////////////////////////////////////////////////////////////////
union TWSR_REG {
    uint8_t all;
    struct {
        uint8_t CLOCK_PRESCALER:2;
        uint8_t RSVD:1;
        uint8_t STATUS:5;
    };
};

union TWAR_REG {
    uint8_t all;
    struct {
        uint8_t GENERAL_CALL_RECOGNITION_ENABLE:1;
        uint8_t SLAVE_ADDRESS:7;
    };
};

union TWCR_REG {
    uint8_t all;
    struct {
        uint8_t INTERRUPT_ENABLE:1;
        uint8_t RSVD:1;
        uint8_t TWI_ENABLE:1;
        uint8_t WRITE_COLLISION_FLAG:1;
        uint8_t STOP_CONDITION:1;
        uint8_t START_CONDITION:1;
        uint8_t ACKNOWLEDGE_ENABLE:1;
        uint8_t INTERRUPT_FLAG:1;
    };
};

struct TWI_t {
    uint8_t bitRate;
    union TWSR_REG status;
    union TWAR_REG slaveAddress;
    uint8_t data;
    union TWCR_REG control;
    uint8_t slaveAddressMask;
};

extern volatile struct TWI_t* TWI;


//////////////////////////////////////////////////////////////////////////
// UART Registers  0xC0, 0xC8
//////////////////////////////////////////////////////////////////////////
union UCSRA_REG {
    uint8_t all;
    struct {
        uint8_t MULTI_PROCESSOR_MODE:1;
        uint8_t SPEED_DOUBLE:1;
        uint8_t PARITY_ERROR:1;
        uint8_t DATA_OVERRUN:1;
        uint8_t FRAME_ERROR:1;
        uint8_t DATA_REGISTER_EMPTY:1;  //UDR0 empty
        uint8_t TRANSMIT_COMPLETE:1;
        uint8_t RECEIVE_COMPLETE:1; //unread data in RXC0
    };
};

union UCSRB_REG {
    uint8_t all;
    struct {
        uint8_t TRANSMIT_DATA_BIT_8:1; //must be written before UDR0
        uint8_t RECEIVE_DATA_BIT_8:1; //must be read before UDR0
        uint8_t CHARACTER_SIZE_H:1; //1=9bit, 0=other
        uint8_t TRANSMITTER_ENABLE:1;
        uint8_t RECEIVER_ENABLE:1;
        uint8_t DATA_REGISTER_EMPTY_INTERRUPT_ENABLE:1;
        uint8_t TRANSMIT_COMPLETE_INTERRUPT_ENABLE:1;
        uint8_t RECEIVE_COMPLETE_INTERRUPT_ENABLE:1;
    };
};

union UCSRC_REG {
    uint8_t all;
    struct {
        uint8_t CLOCK_POLARITY:1;
        uint8_t CHARACTER_SIZE_L:2; //00=5, 01=6, 10=7, 11=8 or 9 (depending on CHARACTER_SIZE_H)
        uint8_t STOP_BIT_SELECT:1; //0=1bit, 1=2bit
        uint8_t PARITY_MODE:2; //10=even, 11=odd
        uint8_t MODE_SELECT:2; //00=asynchronous UART, 01=synchronous UART, 11=SPI Master
    };
};

struct UART_t {
    union UCSRA_REG registerA;
    union UCSRB_REG registerB;
    union UCSRC_REG registerC;
    uint8_t RSVD;
    union {
        uint16_t baud;
        struct {
            uint8_t baudLow;
            uint8_t baudHigh;
        };
    };
    uint8_t data;
};

extern volatile struct UART_t* UART0;
extern volatile struct UART_t* UART1;


void RegistersInit();

void RegistersReset();


#endif /* REGISTERS_H_ */