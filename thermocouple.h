#ifndef THERMOCOUPLE_H_
#define THERMOCOUPLE_H_

#include <stdint.h>

// Configuration
#define THERMOCOUPLE_ADC_CH       3   // P1.3 (A3)
#define FLAME_THRESHOLD_ADC     500   // Empirical ADC threshold
#define SAMPLE_BUFFER_SIZE       5    // Moving average filter size

// Function Prototypes
void Thermocouple_Init(void);
uint8_t Thermocouple_FlameDetected(void);

#endif 