#include "SENSORS.h"
#include <msp430.h>
#include <stdint.h>

// Global variables
volatile char ADCFinished = 0;
volatile unsigned int ADCResult = 0;

// Constants
#define FLAME_THRESHOLD 300  // Temperature threshold in °C

// Pin definitions
#define THERMOCOUPLE_PIN 3   // P1.3 (A3)
#define POT_PIN          4   // P1.4 (A4)
#define THERMISTOR_PIN   5   // P1.5 (A5)

// Function to initialize ADC
void initADC(void) {
    // Configure ADC Pins
    P1SEL0 |= BIT3 | BIT4 | BIT5;  // Select analog function for P1.3, P1.4, P1.5
    P1SEL1 |= BIT3 | BIT4 | BIT5;  // Select analog function for P1.3, P1.4, P1.5

    // Configure ADC
    ADCCTL0 &= ~ADCON;              // Turn off ADC before configuration
    ADCCTL0 |= ADCSHT_2;            // S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;              // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;             // Clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;            // 12-bit conversion results
    ADCIE |= ADCIE0;                // Enable ADC conv complete interrupt
    
    ADCCTL0 |= ADCON;               // Turn on ADC
}

// Function to read ADC value from a specific channel
unsigned int readADC(char Channel) {
    // Clear previous channel selection
    ADCMCTL0 &= ~ADCINCH_15;
    
    // Select the appropriate channel
    switch(Channel) {
        case 3:     // Thermocouple
            ADCMCTL0 |= ADCINCH_3;
            break;
        case 4:     // Potentiometer
            ADCMCTL0 |= ADCINCH_4;
            break;
        case 5:     // Thermistor
            ADCMCTL0 |= ADCINCH_5;
            break;
        default:
            ADCMCTL0 |= ADCINCH_3;  // Default to thermocouple
            break;
    }
    
    // Start conversion
    ADCFinished = 0;
    ADCCTL0 |= ADCENC | ADCSC;    // Sampling and conversion start
    while(ADCFinished != 1);      // Wait until reading is finished
    return ADCResult;             // Return the contents of ADCMEM0
}

// Function to read thermistor and convert to temperature
int16_t therm_Read(void) {
    int adcValue = readADC(THERMISTOR_PIN);
    
    // Avoid division by zero
    if (adcValue <= 0) adcValue = 1;
    
    float resistance = SERIES_RESISTOR * (4095.0 / (float)adcValue - 1.0);
    
    float steinhart = log(resistance / NOMINAL_RESISTANCE) / B_COEFFICIENT;
    steinhart += 1.0 / (NOMINAL_TEMP + 273.15);
    float temperatureK = 1.0 / steinhart;
    float temperatureC = temperatureK - 273.15;
    
    return (int16_t)(temperatureC * 100);  // Return temperature in 0.01°C units
}

// Wrapper function for thermistor reading
unsigned int readThermistor(void) {
    return (unsigned int)therm_Read();
}

// Function to read thermocouple and convert to temperature
unsigned int readThermocouple(void) {
    unsigned int adc_result = readADC(THERMOCOUPLE_PIN);
    float voltage = (adc_result * 3.3f) / 4095.0f;  // Convert to voltage (3.3V reference)
    float temperature = (voltage - 0.001f) / 0.00004f;  // Type K thermocouple: ~40µV/°C
    
    return (unsigned int)(temperature * 100);  // Return temperature in 0.01°C units
}

// Function to detect flame based on thermocouple reading
char flame_Detect(void) {
    unsigned int temp_x100 = readThermocouple();
    float temperature = temp_x100 / 100.0f;
    
    if (temperature > FLAME_THRESHOLD) {
        return 1;  // Flame detected
    } else {
        return 0;  // No flame detected
    }
}

// Function to read potentiometer value
unsigned int readPot(void) {
    unsigned int result = readADC(POT_PIN);
    
    // Scale the potentiometer reading if needed (0-4095 to 0-100)
    unsigned int percent = (result * 100) / 4095;
    
    return percent;  // Return as percentage
}

// Initialize thermistor
void therm_Init(void) {
}

// Initialize flame sensor (thermocouple)
void flame_Init(void) {
}

// Initialize potentiometer
void pot_Init(void) {
}

// ADC interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void) {
    switch(__even_in_range(ADCIV, ADCIV_ADCIFG)) {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            // ADC overflow
            break;
        case ADCIV_ADCTOVIFG:
            // ADC timing overflow
            break;
        case ADCIV_ADCHIIFG:
            // Window comparator high interrupt
            break;
        case ADCIV_ADCLOIFG:
            // Window comparator low interrupt
            break;
        case ADCIV_ADCINIFG:
            // ADC inside window interrupt
            break;
        case ADCIV_ADCIFG:
            // Conversion complete
            ADCResult = ADCMEM0;
            ADCFinished = 1;
            break;
        default:
            break;
    }
}