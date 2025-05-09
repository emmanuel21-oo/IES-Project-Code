#include "intrinsics.h"
#include <msp430.h>
void setRGB(int red, int green, int blue);
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P6DIR |= BIT0 | BIT1 | BIT2;                     // P6.0 and P6.1 and 6.2 output
    P6SEL0 |= BIT0 | BIT1 | BIT2;                    // P6.0, 6.1, and 6.2 to use the timer, set to 00 by default
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2);                 // This is a redundancy check to ensure P6Sel1 is 0 for our set pins that are using tb3.



   P2DIR |= BIT0;
    P2SEL0 |= BIT0;     // Select primary peripheral function (TB1.1)
    P2SEL1 &= ~BIT0;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    TB3CCR0 = 1000-1;                         // PWM Period
    TB3CCTL1 = OUTMOD_3;                      // CCR1 reset/set
    TB3CCR1 = 500;                            // CCR1 PWM duty cycle Red
    TB3CCTL2 = OUTMOD_3;                      // CCR2 reset/set
    TB3CCR2 = 0;                            // CCR2 PWM duty cycle Green
    TB3CCTL3 = OUTMOD_3;                       
    TB3CCR3 = 500;                          //Blue
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR

    
    while (1)
    {
        setRGB(750, 0, 0);
         TB1CCR0 = 320;                             // ~20ms PWM period (ACLK ~32.768kHz) jk my words, this is ~51 hz 
    TB1CCTL1 = OUTMOD_6 | CLLD_2;              // TB1.1 toggle/set
    TB1CCR1 = 310;                              // ~1ms pulse width (5% duty) this gave you exactly 304 = 5% duty. 310=2.12
    TB1CTL = TBSSEL__ACLK | MC_3;              // ACLK, up-down mode
    __delay_cycles(1000000);
        setRGB(0,750,0);
        TB1CCR1 = 288;                              // ~1ms pulse width (5% duty) this gave you exactly 304 = 5% duty. 310=2.12
    TB1CTL = TBSSEL__ACLK | MC_3;              // ACLK, up-down mode
    __delay_cycles(1000000);
        setRGB(0,0,750);
        TB1CCR1 = 300;                              // ~1ms pulse width (5% duty) this gave you exactly 304 = 5% duty. 310=2.12
    TB1CTL = TBSSEL__ACLK | MC_3;              // ACLK, up-down mode

        __delay_cycles(1000000);
    }

    

}


void setRGB(int red, int green, int blue)
{
    TB3CCR1  = red;
    TB3CCR2  = green;
    TB3CCR3  = blue;
}