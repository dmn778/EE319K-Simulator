// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 9/2/2019
// Student names: Dat Nguyen and Neil Narvekar
// Last modification date: 12/05/2019

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

uint32_t Convert(uint32_t input);
uint32_t xADC_In(void);
uint32_t yADC_In(void);
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*PeriodicTask)(void);   // user function

int32_t status;




// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	SYSCTL_RCGCGPIO_R |= 0x08; // port D clock
	while((SYSCTL_RCGCGPIO_R & 0x08) != 0x08){}
		GPIO_PORTD_DIR_R &= 0x0C; //pd2,pd3 input
		GPIO_PORTD_DEN_R &= ~0x0C; //pd2 enable
		GPIO_PORTD_AFSEL_R |= 0x0C;
		GPIO_PORTD_AMSEL_R |= 0x0C;
		
		SYSCTL_RCGCADC_R |= 0x01; // Activate ADC0
		while((SYSCTL_RCGCADC_R & 0x01) != 0x01){}
		ADC0_PC_R |= 0x01; // set sampling rate to 125K Hz
		ADC0_SSPRI_R = 0x0123; // sequencer 3 is highest priority
		ADC0_ACTSS_R &= 0x08; // disable sample sequencer 3
		ADC0_EMUX_R &= ~0xF000; // sequence 3 is s/w trigger
		ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 5 + 0; //Ain5 (PD2)
		ADC0_SSCTL3_R = 0x0006; // no TS0,D0, yes IE0 ENDO
		ADC0_IM_R &= ~0x0008; //Diable SS3 interrupts
		ADC0_ACTSS_R |= 0x08; //Enable rs 3
		ADC0_SAC_R = 0x06; // Avg samples to get better distance values near end of ptentometer
		
		SYSCTL_RCGCADC_R |= 0x02; // Activate ADC1
		while((SYSCTL_RCGCADC_R & 0x02) != 0x02){}
		ADC1_PC_R |= 0x01; // set sampling rate to 125K Hz
		ADC1_SSPRI_R = 0x0120; // sequencer 3 is highest priority
		ADC1_ACTSS_R &= 0x08; // disable sample sequencer 3
		ADC1_EMUX_R &= ~0xF000; // sequence 3 is s/w trigger
		ADC1_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 4; //Ain4(PD3)
		ADC1_SSCTL3_R = 0x0006; // no TS0,D0, yes IE0 ENDO
		ADC1_IM_R &= ~0x0008; //Diable SS3 interrupts
		ADC1_ACTSS_R |= 0x08; //Enable rs 3
		ADC1_SAC_R = 0x06; // Avg samples to get better distance values near end of ptentometer
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2 and PD3
uint32_t xADC_In(void){  // is this the mail function??
	// start sample capture
	// if flag busy, loop back
	// if done, read sample
	// clear flag
	int32_t result;
	//while(status == -1){}
  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result (PD2)
	//while(status == 1){}
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;
}

uint32_t yADC_In(void){  // is this the mail function??
	// start sample capture
	// if flag busy, loop back
	// if done, read sample
	// clear flag
	int32_t result;
	//while(status == -1){}
  ADC1_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC1_RIS_R&0x08)==0){};   // 2) wait for conversion done
  result = ADC1_SSFIFO3_R&0xFFF;   // 3) read result (PD3)
	//while(status == 1){}
  ADC1_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;

}

//------SYSTIC INIT------
void SysTick_Init(void){
	uint32_t period = 0x0FFFFF;
	NVIC_ST_CTRL_R =0;
	NVIC_ST_RELOAD_R = period-1; // use period for rate
	NVIC_ST_CURRENT_R =0; //clear current
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
	NVIC_ST_CTRL_R = 0x7; // enable systic w/ interrupt
  
}
//------TIMER0A INIT-------------
void Timer0A_Init(uint32_t period){long sr;
  sr = StartCritical(); 
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  //PeriodicTask = task;          // user function
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
  EndCritical(sr);
}

//------TIMER1A INIT-------------
void Timer1A_Init(uint32_t period){long sr;
  sr = StartCritical(); 
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  //PeriodicTask = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
  EndCritical(sr);
}

