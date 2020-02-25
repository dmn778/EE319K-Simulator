// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 9/2/19 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "Images.h"
#include "../inc/tm4c123gh6pm.h"

uint32_t input;

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	// make PB0-PB5 outputs on DAC
	SYSCTL_RCGCGPIO_R |= 0x02; // turn on port B clock
	while((SYSCTL_RCGCGPIO_R & 0x2) != 0x2) {} // wait till it is set
	GPIO_PORTB_DIR_R |= 0x3F; // set output PB0-PB5
	GPIO_PORTB_DEN_R |= 0x3F; // enable PB0-PB5
	}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	//data = data*3.3/15;
	GPIO_PORTB_DATA_R = data;
}

