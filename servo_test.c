#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                  // Stop WDT

    // ===== Configure P1.6, P1.7 for TB0.1 and TB0.2 =====
    P1DIR |= BIT6 | BIT7;
    P1SEL1 |= BIT6 | BIT7;

    // ===== Configure P2.0 for TB1.1 =====
    P2DIR |= BIT0;
    P2SEL0 |= BIT0;     // Select primary peripheral function (TB1.1)
    P2SEL1 &= ~BIT0;

    // ===== Unlock GPIOs from high-Z mode =====
    PM5CTL0 &= ~LOCKLPM5;

    // ===== Timer_B0 Setup (P1.6, P1.7) =====
    TB0CCR0 = 320;                             // PWM Period/2
    TB0CCTL1 = OUTMOD_2 | CLLD_2;              // TB0.1 toggle/reset
    TB0CCR1 = 288;                              // Duty cycle   // 9.99% 
    TB0CCTL2 = OUTMOD_6 | CLLD_2;              // TB0.2 toggle/set
    TB0CCR2 = 288;
    TB0CTL = TBSSEL__ACLK | MC_3 | TBCLGRP_1;  // ACLK, up-down, grouped

    // ===== Timer_B1 Setup (P2.0 as TB1.1) =====
    TB1CCR0 = 320;                             // ~20ms PWM period (ACLK ~32.768kHz) jk my words, this is ~51 hz 
    TB1CCTL1 = OUTMOD_6 | CLLD_2;              // TB1.1 toggle/set
    TB1CCR1 = 310;                              // ~1ms pulse width (5% duty) this gave you exactly 304 = 5% duty. 310=2.12
    TB1CTL = TBSSEL__ACLK | MC_3;              // ACLK, up-down mode

    __bis_SR_register(LPM3_bits);              // Enter low power mode

    return 0;
}