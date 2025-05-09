#include "thermocouple.h"
#include <msp430.h>

// Debug variables
volatile uint16_t rawADCValue = 0;
volatile uint16_t filteredValue = 0;

static uint16_t ReadADC(void) {
    ADCMCTL0 = ADCINCH_3;             // Select channel A3
    ADCCTL0 |= ADCENC | ADCSC;        // Start conversion
    while (!(ADCCTL1 & ADCIFG));      // Wait for completion
    ADCCTL1 &= ~ADCIFG;               // Clear flag
    rawADCValue = ADCMEM0;            // Store for debugging
    return rawADCValue;
}

static uint16_t ApplyFilter(void) {
    static uint16_t samples[SAMPLE_BUFFER_SIZE] = {0};
    static uint8_t sampleIndex = 0;
    uint32_t sum = 0;
    uint8_t i;
    
    // Update circular buffer (using bitmask instead of modulo)
    samples[sampleIndex] = ReadADC();
    sampleIndex = (sampleIndex + 1) & (SAMPLE_BUFFER_SIZE - 1);
    
    // Calculate moving average (counting down for ULP)
    for(i = SAMPLE_BUFFER_SIZE; i > 0; i--) {
        sum += samples[i-1];
    }
    
    filteredValue = (uint16_t)(sum >> 2); // Divide by 4 (for SAMPLE_BUFFER_SIZE=5)
    return filteredValue;
}

void Thermocouple_Init(void) {
    // Configure ADC pin
    P1SEL0 |= BIT3;
    P1SEL1 |= BIT3;
    
    // Configure ADC
    ADCCTL0 = ADCSHT_8 | ADCON;
    ADCCTL1 = ADCSHP;
    ADCCTL2 = ADCRES_2;
    ADCMCTL0 = ADCINCH_3;
}

uint8_t Thermocouple_FlameDetected(void) {
    uint16_t adcValue = ApplyFilter();
    return (adcValue > FLAME_THRESHOLD_ADC) ? 1 : 0;
}