// ADC.h
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0

// Student names: Dat Nguyen and Neil Narvekar
// Last modification date: 12/05/2019
#ifndef ADC_H
#define ADC_H
#include <stdint.h>

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void);

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void);
#endif
