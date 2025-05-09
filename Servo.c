#include <msp430.h>

// Servo Configuration (for 16MHz SMCLK with /8 divider)
#define SERVO_PIN         BIT0       // P2.0 (TB1.1)
#define PWM_PERIOD        40000      // 20ms period (16MHz/8 = 2MHz → 2000000Hz → 40000 ticks = 20ms)
#define MIN_PULSE_WIDTH   1000       // 500μs pulse (1000 ticks at 2MHz)
#define MAX_PULSE_WIDTH   2000       // 1000μs pulse (2000 ticks at 2MHz)
#define NEUTRAL_POSITION  1500       // 750μs pulse (1500 ticks)

// Function Prototypes
void Servo_Init(void);
void Servo_SetPosition(unsigned int position);
void Servo_Calibrate(unsigned int min, unsigned int max);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;        // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;            // Unlock GPIOs
    
    Servo_Init();                    // Initialize servo control
    
    // Example usage:
    while(1) {
        Servo_SetPosition(NEUTRAL_POSITION);  // 750μs pulse
        __delay_cycles(1000000);     // Hold for 1s
        
        Servo_SetPosition(MIN_PULSE_WIDTH);   // 500μs pulse
        __delay_cycles(1000000);
        
        Servo_SetPosition(MAX_PULSE_WIDTH);   // 1000μs pulse
        __delay_cycles(1000000);
    }
}

// Initialize servo PWM on P2.0
void Servo_Init(void) {
    // Configure P2.0 for TB1.1 output
    P2DIR |= SERVO_PIN;
    P2SEL0 |= SERVO_PIN;             // Select TB1.1 function
    P2SEL1 &= ~SERVO_PIN;
    
    // Timer_B1 configuration (Servo control)
    TB1CCR0 = PWM_PERIOD;            // 20ms period
    TB1CCTL1 = OUTMOD_7;             // Reset/set output mode
    TB1CCR1 = NEUTRAL_POSITION;      // Start at neutral position
    TB1CTL = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR; // SMCLK/8, up mode
}

// Set servo pulse width in microseconds (500-1000μs)
void Servo_SetPosition(unsigned int pulse_us) {
    // Convert microseconds to timer ticks (2MHz clock → 2 ticks/μs)
    unsigned int position = pulse_us * 2;
    
    // Constrain position to valid range
    if(position < MIN_PULSE_WIDTH) position = MIN_PULSE_WIDTH;
    if(position > MAX_PULSE_WIDTH) position = MAX_PULSE_WIDTH;
    
    TB1CCR1 = position;  // Update PWM pulse width
}

// Calibrate servo limits in microseconds
void Servo_Calibrate(unsigned int min_us, unsigned int max_us) {
    // Convert to timer ticks
    unsigned int min_ticks = min_us * 2;
    unsigned int max_ticks = max_us * 2;
    
    // Update min/max pulse widths
    #undef MIN_PULSE_WIDTH
    #undef MAX_PULSE_WIDTH
    #define MIN_PULSE_WIDTH min_ticks
    #define MAX_PULSE_WIDTH max_ticks
    
    // Safety check
    if(MIN_PULSE_WIDTH >= MAX_PULSE_WIDTH) {
        #undef MIN_PULSE_WIDTH
        #undef MAX_PULSE_WIDTH
        #define MIN_PULSE_WIDTH 1000  // Default 500μs
        #define MAX_PULSE_WIDTH 2000  // Default 1000μs
    }
}