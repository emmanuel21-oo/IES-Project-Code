#include "main_valve.h"
#include <msp430.h>

void MainValve_Init(void) {
    // Configure PWM pin
    P2DIR |= MAIN_VALVE_PWM_PIN;
    P2SEL0 |= MAIN_VALVE_PWM_PIN;      // Select TB1.1 function
    P2SEL1 &= ~MAIN_VALVE_PWM_PIN;
    
    // Timer_B1 configuration
    TB1CCR0 = MAIN_VALVE_PWM_PERIOD;    // 20ms period
    TB1CCTL1 = OUTMOD_7;                // Reset/set output mode
    TB1CTL = TBSSEL__SMCLK | MC__UP | TBCLR; // SMCLK, up mode
    
    // Start with valve closed
    TB1CCR1 = MAIN_VALVE_MIN_FLOW;
}

void MainValve_Set(uint8_t flow_percent) {
    // Constrain input to 0-100%
    if(flow_percent > 100) flow_percent = 100;
    
    // Calculate pulse width (linear 1-2ms)
    uint16_t pulse_width = MAIN_VALVE_MIN_FLOW + 
                          ((MAIN_VALVE_MAX_FLOW - MAIN_VALVE_MIN_FLOW) * flow_percent) / 100;
    
    // Update PWM duty cycle
    TB1CCR1 = pulse_width;
}