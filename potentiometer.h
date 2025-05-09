#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#include <msp430.h>
#include <stdint.h>

// Function prototypes
void Pot_Init(void);
int16_t Pot_Read(void);

#endif 