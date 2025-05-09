#include <msp430.h>
#include <stdio.h>
#include "thermistor.h"


// Global variables for ADC (required by thermistor.c)
char ADCFinished = 0;
unsigned int ADCResult = 0;

// Function prototypes
void initSystem(void);
void delay(unsigned int ms);

// ADC interrupt service routine
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void)
{
    switch(__even_in_range(ADCIV, ADCIV_ADCIFG))
    {
        case ADCIV_ADCIFG:
            ADCResult = ADCMEM0;
            ADCFinished = 1;
            break;
        default:
            break;
    }
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    // Initialize system
    initSystem();
    
    // Initialize thermistor ADC
    thermistor_InitADC();
    
    // Enable global interrupts
    __enable_interrupt();
    
    // Variables for temperature readings
    uint16_t tempRaw;
    float tempDegC;
    
    while(1)
    {
        // Read temperature from thermistor
        tempRaw = thermistor_ReadTemp();
        
        // Convert from 0.1°C units to actual degrees
        tempDegC = (float)tempRaw / 10.0f;
        
        delay(1000);  // 1 second between readings
    }
    
    return 0;
}

// Basic system initialization
void initSystem(void)
{

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    
}

// Simple delay function
void delay(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 100; j++);  // Adjust based on your CPU frequency
}