#ifndef MAIN_VALVE_H_
#define MAIN_VALVE_H_

#include <stdint.h>

// Main Valve Configuration
#define MAIN_VALVE_PWM_PIN     BIT0       // P2.0 (TB1.1)
#define MAIN_VALVE_PWM_PERIOD  20000      // 20ms period (standard for flow control)
#define MAIN_VALVE_MIN_FLOW    1000       // 1ms pulse (5% duty)
#define MAIN_VALVE_MAX_FLOW    2000       // 2ms pulse (10% duty)

// Function Prototypes
void MainValve_Init(void);
void MainValve_Set(uint8_t flow_percent);  // 0-100% flow rate

#endif