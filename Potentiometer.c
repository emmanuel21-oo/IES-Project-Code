#include "potentiometer.h"
#include <msp430.h>

// Hardware Configuration
#define POT_ADC_CHANNEL   4       // P1.4 (A4)
#define POT_MIN_ADC       100     // Minimum expected ADC value (0% position)
#define POT_MAX_ADC       4095    // Maximum expected ADC value (100% position)

void Pot_Init(void) {
    // Configure ADC pin (P1.4)
    P1SEL0 |= BIT4;
    P1SEL1 |= BIT4;
    
    // Configure ADC (12-bit, single-channel)
    ADCCTL0 = ADCSHT_8 | ADCON;        // 96-cycle sample, ADC on
    ADCCTL1 = ADCSHP;                  // Use sampling timer
    ADCCTL2 = ADCRES_2;                // 12-bit resolution
    ADCMCTL0 = ADCINCH_4;              // Select channel A4
}

int16_t Pot_Read(void) {
    ADCCTL0 |= ADCENC | ADCSC;         // Start conversion
    while (!(ADCCTL1 & ADCIFG));       // Wait for completion
    ADCCTL1 &= ~ADCIFG;                // Clear flag
    
    uint16_t adcValue = ADCMEM0;
    
    // Constrain the ADC reading to expected range
    if (adcValue < POT_MIN_ADC) adcValue = POT_MIN_ADC;
    if (adcValue > POT_MAX_ADC) adcValue = POT_MAX_ADC;
    
    // Convert to percentage (0-100%)
    int16_t setpoint = (int16_t)((adcValue - POT_MIN_ADC) * 100L / (POT_MAX_ADC - POT_MIN_ADC));
    
    return setpoint;
}