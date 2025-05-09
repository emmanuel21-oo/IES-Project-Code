#include <msp430.h>

#define PILOT_VALVE_PIN  BIT3   // P1.3 - Control pin for pilot valve
#define HEAT_STATUS_PIN  BIT4   // P1.4 - Optional status indicator

// Function prototypes
char Pilot_open(void);
void Heat_On(void);
void Pilot_Close(void);
void Pilot_Init(void);

// Global variable to track valve state
extern volatile char pilotValveOpen = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Disable GPIO power-on default
    
    // Configure input pins
    P5DIR &= ~BIT0;    // P5.0 as input (thermostat)
    P1DIR &= ~(BIT1 | BIT2);  // P1.1 and P1.2 as inputs
    
    Pilot_Init();                 // Initialize pilot valve control
    
    while(1) {
        // Control logic
        if (P5IN & BIT0) {        // If thermostat calls for heat
            Heat_On();            // Open valve if not already open
        }
        else if (!(P1IN & BIT1) || (P1IN & BIT2)) {  // Emergency stop conditions
            Pilot_Close();        // Force close valve
        }
        
        __delay_cycles(100000);   // 100ms delay
    }
}


// Initialize pilot valve GPIO
void Pilot_Init(void) {
    P1DIR |= PILOT_VALVE_PIN | HEAT_STATUS_PIN;  // Set as outputs
    P1OUT &= ~(PILOT_VALVE_PIN | HEAT_STATUS_PIN); // Start with valve closed
    pilotValveOpen = 0;  // Initialize state
}

// Close pilot valve (force close regardless of current state)
void Pilot_Close(void) {
    P1OUT &= ~PILOT_VALVE_PIN;   // Close valve
    P1OUT &= ~HEAT_STATUS_PIN;    // Turn off status indicator
    pilotValveOpen = 0;           // Update state
}

// Toggle pilot valve state (open/close)
char Pilot_open(void) {
    if (pilotValveOpen) {
        Pilot_Close();
        return 0;
    }
    else {
        P1OUT |= PILOT_VALVE_PIN;    // Open valve
        P1OUT |= HEAT_STATUS_PIN;    // Turn on status indicator
        pilotValveOpen = 1;
        return 1;
    }
}

// Turn on heat (opens pilot valve if not already open)
void Heat_On(void) {
    if (!pilotValveOpen) {
        P1OUT |= PILOT_VALVE_PIN;    // Open valve
        P1OUT |= HEAT_STATUS_PIN;    // Turn on status indicator
        pilotValveOpen = 1;
    }
}