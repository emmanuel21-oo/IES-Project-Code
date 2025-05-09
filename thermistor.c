#include "msp430.h"
#include "thermistor.h"
#include <math.h> 

extern char ADCFinished;
extern unsigned int ADCResult;

static unsigned int readADC(char Channel) {
    switch(Channel) {
        case 4: ADCMCTL0 = ADCINCH_4; break; // Only thermistor channel
        default: ADCMCTL0 = ADCINCH_4; break; // Fallback to thermistor
    }
    
    ADCFinished = 0;
    ADCCTL0 |= ADCENC | ADCSC;
    while(ADCFinished != 1);
    return ADCResult;
}

void thermistor_InitADC() {
    // Configure ONLY the thermistor pin (P1.4)
    P1SEL0 |= BIT4;
    P1SEL1 |= BIT4;

    // ADC Configuration (matches your style)
    ADCCTL0 |= ADCSHT_2 | ADCON;
    ADCCTL1 |= ADCSHP;
    ADCCTL2 = (ADCCTL2 & ~ADCRES) | ADCRES_2;
    ADCIE |= ADCIE0;
}

uint16_t thermistor_ReadTemp() {
    uint16_t adcValue = readADC(THERMISTOR_ADC_CH);
    if (adcValue == 0) adcValue = 1; // Avoid division by zero

    // Convert to resistance
    float resistance = SERIES_RESISTOR * (4095.0f / adcValue - 1.0f);

    // Steinhart-Hart equation 
    float tempK = 1.0f / (logf(resistance / THERMISTOR_NOMINAL)/THERMISTOR_BETA + 1.0f/(25.0f + 273.15f));
    float tempC = tempK - 273.15f;

    return (uint16_t)(tempC * 10); // Return as 0.1°C units (e.g., 250 = 25.0°C)
}