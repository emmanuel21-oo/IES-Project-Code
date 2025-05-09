#include <msp430.h> 

// System state enumeration
enum system_state {IDLE, HEATING} state;

// Define LED pins based on your specifications
#define GREEN_LED BIT6  // P6.6 (Green LED)
#define RED_LED   BIT0  // P1.0 (Red LED)

// Define button pins
#define HEAT_BTN1 BIT1  // P4.1 (First button)
#define HEAT_BTN2 BIT3  // P2.3 (Second button)

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;       // Disable GPIO power-on default
    
    // Initialize LEDs (on different ports now)
    P6DIR |= GREEN_LED;         // Set P6.6 as output for Green LED
    P1DIR |= RED_LED;           // Set P1.0 as output for Red LED
    P6OUT &= ~GREEN_LED;        // Initially Green LED off
    P1OUT &= ~RED_LED;          // Initially Red LED off
    
    // Configure button 1 (P4.1)
    P4DIR &= ~HEAT_BTN1;        // Set as input
    P4REN |= HEAT_BTN1;         // Enable pull-up/down
    P4OUT |= HEAT_BTN1;         // Pull-up resistor
    P4IES |= HEAT_BTN1;         // High-to-low transition interrupt
    P4IE |= HEAT_BTN1;          // Enable interrupt
    P4IFG &= ~HEAT_BTN1;        // Clear any pending interrupt
    
    // Configure button 2 (P2.3)
    P2DIR &= ~HEAT_BTN2;        // Set as input
    P2REN |= HEAT_BTN2;         // Enable pull-up/down
    P2OUT |= HEAT_BTN2;         // Pull-up resistor
    P2IES |= HEAT_BTN2;         // High-to-low transition interrupt
    P2IE |= HEAT_BTN2;          // Enable interrupt
    P2IFG &= ~HEAT_BTN2;        // Clear any pending interrupt
    
    // Initialize system state
    state = IDLE;
    
    // Main state machine loop
    while(1)
    {
        switch(state)
        {
            case IDLE:
                P6OUT |= GREEN_LED;     // Green LED on (P6.6)
                P1OUT &= ~RED_LED;      // Red LED off (P1.0)
                break;
                
            case HEATING:
                P1OUT |= RED_LED;       // Red LED on (P1.0)
                P6OUT &= ~GREEN_LED;    // Green LED off (P6.6)
                break;
        }
        
        __bis_SR_register(LPM0_bits | GIE); // Enter LPM0 with interrupts
        __no_operation();                   // For debugger
    }
}

// Port 4 interrupt service routine (for button 1)
#pragma vector=PORT4_VECTOR
__interrupt void Port_4_ISR(void)
{
    if(P4IFG & HEAT_BTN1)  // Check if button 1 triggered the interrupt
    {
        // Toggle edge sensitivity (detect both rising and falling)
        P4IES ^= HEAT_BTN1;
        
        // Change state based on current button level
        if(P4IN & HEAT_BTN1) {
            state = IDLE;       // Rising edge (button released)
        } else {
            state = HEATING;    // Falling edge (button pressed)
        }
        
        // Wake up from LPM0
        __bic_SR_register_on_exit(LPM0_bits);
        
        P4IFG &= ~HEAT_BTN1;    // Clear interrupt flag
    }
}

// Port 2 interrupt service routine (for button 2)
#pragma vector=PORT2_VECTOR
__interrupt void Port_2_ISR(void)
{
    if(P2IFG & HEAT_BTN2)  // Check if button 2 triggered the interrupt
    {
        // Toggle edge sensitivity (detect both rising and falling)
        P2IES ^= HEAT_BTN2;
        
        // Change state based on current button level
        if(P2IN & HEAT_BTN2) {
            state = IDLE;       // Rising edge (button released)
        } else {
            state = HEATING;    // Falling edge (button pressed)
        }
        
        // Wake up from LPM0
        __bic_SR_register_on_exit(LPM0_bits);
        
        P2IFG &= ~HEAT_BTN2;    // Clear interrupt flag
    }
}