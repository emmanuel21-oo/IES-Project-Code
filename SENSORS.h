#ifndef SENSORS_H_
#define SENSORS_H_

#include <msp430.h>
#include <stdint.h>
#include <math.h>

// Thermistor constants
#define SERIES_RESISTOR     10000
#define NOMINAL_RESISTANCE  10000
#define NOMINAL_TEMP        25
#define B_COEFFICIENT       3950

// Function prototypes
void initADC(void);
unsigned int readADC(char Channel);

// Thermistor functions
void therm_Init(void);
int16_t therm_Read(void);
unsigned int readThermistor(void);

// Thermocouple functions
void flame_Init(void);
unsigned int readThermocouple(void);
char flame_Detect(void);

// Potentiometer functions
void pot_Init(void);
unsigned int readPot(void);

#endif /* SENSORS_H_ */