#ifndef THERMISTOR_H_
#define THERMISTOR_H_

#include <stdint.h>

// Thermistor Parameters (10kΩ NTC)
#define THERMISTOR_ADC_CH  4     // P1.4
#define THERMISTOR_NOMINAL 10000 // 10kΩ @ 25°C
#define THERMISTOR_BETA    3950  // B-coefficient
#define SERIES_RESISTOR    10000 // Voltage divider resistor


void thermistor_InitADC();
uint16_t thermistor_ReadTemp(); 

#endif 