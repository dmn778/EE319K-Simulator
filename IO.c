// IO.c
// This software configures the switch and LED
// You are allowed to use any switch and any LED, 
// although the Lab suggests the SW1 switch PF4 and Red LED PF1
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: March 30, 2018
// Last Modified:  change this or look silly
// Lab number: 7


#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "ST7735.h"

void PortF_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20; // port f clock
	while((SYSCTL_RCGCGPIO_R & 0x20) != 0x20){}
		  GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x01; 
		GPIO_PORTF_DIR_R &= ~0x11; //pf4 input
		GPIO_PORTF_DIR_R |= 0x04; //pf2 heartbeat output
		GPIO_PORTF_DEN_R |= 0x15; //enable 
		GPIO_PORTF_PUR_R |= 0x11; //pull up resistor

}

void PortE_Init(void){
		SYSCTL_RCGCGPIO_R |= 0x10; // port e clock
		while((SYSCTL_RCGCGPIO_R & 0x10) != 0x10){}
		GPIO_PORTE_DIR_R &= ~0x06; //pE1, PE2 input
		GPIO_PORTE_DEN_R |= 0x06; //enable 

}

//------------IO_HeartBeat------------
// Toggle the output state of the  LED.
// Input: none
// Output: none
void IO_HeartBeat(void) {
 // --UUU-- PF2 is heartbeat
	GPIO_PORTF_DATA_R ^=  0x04; //xor for heartbeat
}


//------------IO_Touch------------
// wait for release and press of the switch
// Delay to debounce the switch
// Input: none
// Output: none
uint32_t button(void) {
 // if pressed return 1, else return 0
	if((((GPIO_PORTF_DATA_R & 0x10) == 0x10) && (GPIO_PORTE_DATA_R & 0x02) == 0x02)){
		return 0;
	}
	return 1;
}

uint32_t button2(void) {
	if((((GPIO_PORTF_DATA_R & 0x01) == 0x01) && (GPIO_PORTE_DATA_R & 0x04) == 0x04)){
		return 0;
	}
	return 1;
}
