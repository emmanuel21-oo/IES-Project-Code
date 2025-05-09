/*
 * Electronic Ignition Variable Gas Valve Control Module
 * Main Control System
 * 
 * This file integrates all the subsystems:
 * - Temperature sensing (thermistor/thermocouple)
 * - Heat request detection
 * - Pilot ignition sequence
 * - Main valve control
 * - Status indicators
 */

#include <msp430.h>
#include <stdint.h>
#include "SENSORS.h"
#include "valve_control.h"
#include "thermistor.h"
#include "thermocouple.h"
#include "potentiometer.h"

// System state definitions
typedef enum {
    STATE_IDLE,           // System idle, waiting for heat request
    STATE_PREPURGE,       // Pre-purge sequence
    STATE_PILOT_IGNITION, // Igniting pilot
    STATE_PILOT_PROVE,    // Verifying pilot flame
    STATE_MAIN_VALVE,     // Main valve operation
    STATE_SHUTDOWN,       // Normal shutdown sequence
    STATE_LOCKOUT         // Safety shutdown
} SystemState;

// External function declarations (from other .c files)
extern void Pilot_Init(void);
extern void Pilot_Close(void);
extern void Heat_On(void);
extern char Pilot_open(void);
extern void Pilot_State(char pilot_status);
extern void Igniter_Init(void);

// Hardware pins
#define HEAT_REQUEST_PIN  BIT1  // P4.1 - Heat request input
#define SAFETY_SWITCH_PIN BIT3  // P2.3 - Safety switch
#define STATUS_GREEN_PIN  BIT6  // P6.6 - Green status LED
#define STATUS_RED_PIN    BIT0  // P1.0 - Red status LED
#define IGNITER_LED_PIN   BIT4  // P5.4 - Igniter LED

// Constants
#define PREPURGE_TIME     3000  // Pre-purge time in milliseconds
#define IGNITION_TRIAL    10000 // Max ignition trial time in milliseconds
#define FLAME_PROVE_TIME  1000  // Time to verify stable flame (milliseconds)
#define MAX_TRIALS        3     // Maximum ignition trials before lockout
#define RETRY_DELAY       5000  // Delay between trials (milliseconds)

// Global variables
volatile SystemState currentState = STATE_IDLE;
volatile uint16_t systemTimer = 0;
volatile uint8_t ignitionTrials = 0;
volatile uint8_t pilotValveOpen = 0;
volatile uint8_t mainValveEnabled = 0;

// Function prototypes
void initSystem(void);
void processState(void);
void updateOutputs(void);
void delay_ms(uint16_t ms);
void setStatusLED(uint8_t green, uint8_t red);

int main(void) {
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    // Initialize system
    initSystem();
    
    // Main loop
    while (1) {
        // Process current state
        processState();
        
        // Update physical outputs
        updateOutputs();
        
        // Small delay for debouncing and stability
        delay_ms(10);
    }
}

void initSystem(void) {
    // Disable the GPIO power-on default high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;
    
    // Initialize subsystems
    initADC();              // Initialize ADC
    Thermocouple_Init();    // Initialize thermocouple
    thermistor_InitADC();   // Initialize thermistor
    Pilot_Init();           // Initialize pilot valve
    Igniter_Init();         // Initialize igniter
    Valve_Init();           // Initialize main valve
    Pot_Init();             // Initialize potentiometer
    
    // Configure heat request input pin (P4.1)
    P4DIR &= ~HEAT_REQUEST_PIN;   // Input
    P4REN |= HEAT_REQUEST_PIN;    // Enable pull-up/down
    P4OUT |= HEAT_REQUEST_PIN;    // Pull-up
    P4IES |= HEAT_REQUEST_PIN;    // High to low transition
    P4IE |= HEAT_REQUEST_PIN;     // Enable interrupt
    P4IFG &= ~HEAT_REQUEST_PIN;   // Clear interrupt flag
    
    // Configure safety switch input (P2.3)
    P2DIR &= ~SAFETY_SWITCH_PIN;  // Input
    P2REN |= SAFETY_SWITCH_PIN;   // Enable pull-up/down
    P2OUT |= SAFETY_SWITCH_PIN;   // Pull-up
    P2IES |= SAFETY_SWITCH_PIN;   // High to low transition
    P2IE |= SAFETY_SWITCH_PIN;    // Enable interrupt
    P2IFG &= ~SAFETY_SWITCH_PIN;  // Clear interrupt flag
    
    // Configure status LEDs
    P6DIR |= STATUS_GREEN_PIN;    // Output
    P1DIR |= STATUS_RED_PIN;      // Output
    P6OUT &= ~STATUS_GREEN_PIN;   // Initially off
    P1OUT &= ~STATUS_RED_PIN;     // Initially off
    
    // Configure Timer A0 for 1ms ticks
    TA0CCR0 = 1000-1;            // 1ms @ 1MHz
    TA0CCTL0 = CCIE;             // Enable interrupt
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR | ID__1; // SMCLK, up mode, clear
    
    // Enable global interrupts
    __enable_interrupt();
    
    // Initial state
    currentState = STATE_IDLE;
    setStatusLED(1, 0);  // Green on, Red off in idle
}

void processState(void) {
    static uint16_t stateTimer = 0;
    uint8_t flameDetected = 0;
    
    // Check for flame
    flameDetected = Thermocouple_FlameDetected();
    
    // Process based on current state
    switch (currentState) {
        case STATE_IDLE:
            // Check if heat is requested
            if (!(P4IN & HEAT_REQUEST_PIN)) {
                currentState = STATE_PREPURGE;
                stateTimer = 0;
                setStatusLED(0, 1);  // Green off, Red on during sequence
            }
            break;
            
        case STATE_PREPURGE:
            // Wait for prepurge time
            if (stateTimer >= PREPURGE_TIME) {
                currentState = STATE_PILOT_IGNITION;
                stateTimer = 0;
                ignitionTrials = 0;
            }
            break;
            
        case STATE_PILOT_IGNITION:
            // Open pilot valve and start ignition
            if (stateTimer == 0) {
                Heat_On();  // Open pilot valve
                pilotValveOpen = 1;
                // Igniter on - simulated with LED
                P5OUT |= IGNITER_LED_PIN;
                ignitionTrials++;
            }
            
            // Check if flame is detected
            if (flameDetected) {
                currentState = STATE_PILOT_PROVE;
                stateTimer = 0;
                // Turn off igniter
                P5OUT &= ~IGNITER_LED_PIN;
            }
            
            // Check for timeout
            if (stateTimer >= IGNITION_TRIAL) {
                // Turn off igniter
                P5OUT &= ~IGNITER_LED_PIN;
                
                // Close pilot valve
                Pilot_Close();
                pilotValveOpen = 0;
                
                // Check if we've reached max trials
                if (ignitionTrials >= MAX_TRIALS) {
                    currentState = STATE_LOCKOUT;
                } else {
                    // Wait before retry
                    stateTimer = 0;
                    currentState = STATE_PREPURGE;
                    delay_ms(RETRY_DELAY);
                }
            }
            break;
            
        case STATE_PILOT_PROVE:
            // Verify flame stability for a short period
            if (!flameDetected) {
                // Flame lost during prove period
                currentState = STATE_SHUTDOWN;
                stateTimer = 0;
            } else if (stateTimer >= FLAME_PROVE_TIME) {
                // Flame proven stable, open main valve
                currentState = STATE_MAIN_VALVE;
                stateTimer = 0;
                mainValveEnabled = 1;
                
                // Set main valve flow based on potentiometer
                int16_t setpoint = Pot_Read();
                Valve_Set((uint8_t)setpoint);
                
                // Set status LED to indicate heat active
                setStatusLED(1, 1);  // Both LEDs on during heating
            }
            break;
            
        case STATE_MAIN_VALVE:
            // Normal operation - monitor flame and controls
            
            // Update valve position based on potentiometer
            int16_t setpoint = Pot_Read();
            Valve_Set((uint8_t)setpoint);
            
            // Check if flame is lost
            if (!flameDetected) {
                currentState = STATE_SHUTDOWN;
                stateTimer = 0;
            }
            
            // Check if heat request is removed
            if (P4IN & HEAT_REQUEST_PIN) {
                currentState = STATE_SHUTDOWN;
                stateTimer = 0;
            }
            break;
            
        case STATE_SHUTDOWN:
            // Close main valve immediately
            mainValveEnabled = 0;
            Valve_Set(0);
            
            // Keep pilot valve open briefly to ensure clean shutdown
            if (stateTimer >= 1000) {  // 1 second delay
                Pilot_Close();
                pilotValveOpen = 0;
                currentState = STATE_IDLE;
                stateTimer = 0;
                
                // Return to idle state indicators
                setStatusLED(1, 0);  // Green on, Red off
            }
            break;
            
        case STATE_LOCKOUT:
            // Lockout state - all valves closed, requires manual reset
            Pilot_Close();
            pilotValveOpen = 0;
            mainValveEnabled = 0;
            Valve_Set(0);
            
            // Blink red LED to indicate lockout
            if ((stateTimer % 500) < 250) {
                P1OUT |= STATUS_RED_PIN;
            } else {
                P1OUT &= ~STATUS_RED_PIN;
            }
            P6OUT &= ~STATUS_GREEN_PIN;  // Green off
            
            // Check for reset (both buttons pressed)
            if (!(P4IN & HEAT_REQUEST_PIN) && !(P2IN & SAFETY_SWITCH_PIN)) {
                delay_ms(1000);  // Debounce and confirm
                if (!(P4IN & HEAT_REQUEST_PIN) && !(P2IN & SAFETY_SWITCH_PIN)) {
                    currentState = STATE_IDLE;
                    stateTimer = 0;
                    setStatusLED(1, 0);  // Green on, Red off
                }
            }
            break;
    }
    
    // Update state timer
    stateTimer++;
}

void updateOutputs(void) {
    // Update pilot valve state
    Pilot_State(pilotValveOpen);
    
    // Safety check: if safety switch is triggered, force shutdown
    if (!(P2IN & SAFETY_SWITCH_PIN)) {
        if (currentState != STATE_LOCKOUT && currentState != STATE_IDLE) {
            currentState = STATE_SHUTDOWN;
        }
    }
}

void setStatusLED(uint8_t green, uint8_t red) {
    if (green) {
        P6OUT |= STATUS_GREEN_PIN;
    } else {
        P6OUT &= ~STATUS_GREEN_PIN;
    }
    
    if (red) {
        P1OUT |= STATUS_RED_PIN;
    } else {
        P1OUT &= ~STATUS_RED_PIN;
    }
}

void delay_ms(uint16_t ms) {
    systemTimer = 0;
    while (systemTimer < ms);
}

// Button 1 interrupt (heat request)
#pragma vector=PORT4_VECTOR
__interrupt void Port_4_ISR(void) {
    if (P4IFG & HEAT_REQUEST_PIN) {
        // Toggle interrupt edge
        P4IES ^= HEAT_REQUEST_PIN;
        
        P4IFG &= ~HEAT_REQUEST_PIN;  // Clear interrupt flag
    }
}

// Button 2 interrupt (safety switch)
#pragma vector=PORT2_VECTOR
__interrupt void Port_2_ISR(void) {
    if (P2IFG & SAFETY_SWITCH_PIN) {
        // Toggle interrupt edge
        P2IES ^= SAFETY_SWITCH_PIN;
        
        P2IFG &= ~SAFETY_SWITCH_PIN;  // Clear interrupt flag
    }
}

// Timer A0 interrupt for millisecond timing
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    systemTimer++;
}

// ADC interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void) {
    switch(__even_in_range(ADCIV, ADCIV_ADCIFG)) {
        case ADCIV_ADCIFG:
            ADCResult = ADCMEM0;
            ADCFinished = 1;
            break;
        default:
            break;
    }
}