#include <msp430.h>

// Igniter LED Configuration
#define IGNITER_LED_PIN   BIT4    // P5.4
#define IGNITER_RESISTOR  1500    // 1.5kΩ series resistor

// Function Prototypes
void Igniter_Init(void);
void Pilot_State(char pilot_status);  // 1=pilot lit, 0=pilot off

char pilotValveOpen=0;  // From your pilot valve code

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;        // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;            // Unlock GPIOs
    
    Igniter_Init();                  // Initialize igniter LED
    
    while(1) {
        Pilot_State(pilotValveOpen);  // Sync LED with pilot state
        pilotValveOpen = 1;
        __delay_cycles(10000);       // 100ms refresh rate

        Pilot_State(pilotValveOpen);  // Sync LED with pilot state
        pilotValveOpen = 0;
        __delay_cycles(100000);       // 100ms refresh rate


    }
}

// Initialize igniter LED GPIO
void Igniter_Init(void) {
    P5DIR |= IGNITER_LED_PIN;        // Set P5.4 as output
    P5OUT &= ~IGNITER_LED_PIN;       // Start with LED off
    
    
}

// Control LED based on pilot state
void Pilot_State(char pilot_status) {
    if(pilot_status) {
        P5OUT |= IGNITER_LED_PIN;    // Turn on igniter LED
    } else {
        P5OUT &= ~IGNITER_LED_PIN;   // Turn off igniter LED
    }
}