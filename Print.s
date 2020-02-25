; Print.s
; Student names: Dat Nguyen and Neil Narvekar
; Last modification date: 11/10/2019
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

n EQU 0
	
    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
    ; copy/paste Lab 7 solution here
		AND R3, #0 ; clear counting register
		ADD R3, #1 ; set counting register to 1
ODloop	SUB SP, #8 ; allocate	
		STR R0,[SP,#n] ; n=R0 number, local variable
		
		MOV R2, #10 
		CMP R0, R2
		BHI continue ; check if absolute value of num greater than or = to 10
		BEQ continue ; check if absolute value of num greater than or = to 10
		ADD SP,#8 ; if not, deallocate
		PUSH{R0,R1} ; put digit on stack
		B doneOD
	
		; if number greater or equal to 10, do num %10
continue	UDIV R1, R0, R2 ; divide by 10
			MUL R1, R2 ; multiply by 10, now last digit is 0 (truncated)
			SUB R0, R0, R1 ; subtract original from number with last digit 0 to get n % 10 (ex. 187-180=7)
			
			LDR R2,[SP,#n] ; load back original number to R2 for later
			ADD SP, #8 ; deallocate
	
			PUSH{R0,R1} ; push digit
			ADD R3, #1 ; increment digit counter
	
			MOV R0, R2 ; move original number back
			MOV R1, #10
			UDIV R0, R1 ; divide num by 10 for next iteration, to find next digit
			B ODloop
	
	
doneOD
		CMP R3, #0 
		BLE doneOD2 ; if iterations less than or equal to 0, program done
		
		POP{R0,R1} ; else, pop digit off stack
		ADD R0, #0x30 ; make ascii
		PUSH{R3,LR} ; save registers
		BL ST7735_OutChar ; output ascii 
		POP{R3,LR} ;save registers
		SUB R3, #1 ; decrement digit counter
		B doneOD
	
doneOD2      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
    ; copy/paste Lab 7 solution here
	CMP R0, #1000
	BHI onethoucase ; if absolute value greater or = to 1000, goto special case
	BEQ onethoucase ; if absolute value greater or = to 1000, goto special case
	
		AND R3, #0 ; clear counting register
		ADD R3, #1 ; counting register
OFloop	SUB SP, #8 ; allocate	
		STR R0,[SP,#n] ; n=R0 number, local variable
		
		MOV R2, #10 
		CMP R0, R2
		BHI contOF ; check if num greater than or = to 10
		BEQ contOF ; check if num greater than or = to 10
		
		ADD SP,#8 ; if not deallocate
		PUSH{R0,R1} ; push digit to stack
		B doneOF
	
	; if number greater or equal to 10, do num % 10
contOF	UDIV R1, R0, R2 ; divide by 10
		MUL R1, R2 ; multiply by 10
		SUB R0, R0, R1 ; subtract original from number with last digit 0 to get n % 10
		LDR R2,[SP,#n] ; load back original number to R0 for later
		ADD SP, #8 ; deallocate
	
		PUSH{R0,R1} ; push digit
		ADD R3, #1 ; counter
	
		MOV R0, R2 ; move back
		MOV R1, #10
		UDIV R0, R1 ; divide num by 10 for next iteration
		B OFloop
	
	
	
doneOF	CMP R3, #1
		BEQ onedig ; if 1 digit, go special case
		CMP R3, #2
		BEQ twodig ; if 2 digit, go special case
		B threedig ; else, goto three digit

onedig	; if one dig, push two zeros
		MOV R0, #0x00 
		PUSH{R0,R1}
		PUSH{R0,R1} ; push two zeros
		ADD R3, #2 ;set counter to 3
		B threedig
		
twodig	; if two dig, push 1 zero
		MOV R0, #0x00
		PUSH{R0,R1}
		ADD R3, #1 ; set counter to 3
		B threedig

threedig	CMP R3, #0
			BLE doneOF2 ; if iterations less than or equal to zero, program done
			
outnum		POP{R0,R1} ; pop digit
			ADD R0, #0x30 ; get ascii
			PUSH{R3,LR} ;save registers
			BL ST7735_OutChar ; output
			POP{R3,LR} ; save registers
			
			CMP R3, #3 ; check if three digits
			BNE skipdot ; if not, skip putting a dot
			MOV R0, #0x2E 
			PUSH{R3,LR} ; save registers
			BL ST7735_OutChar ; output
			POP{R3,LR} ;save registers
skipdot		SUB R3, #1 ; decrement counter for digits
			B threedig
		
onethoucase ; if 1000 or greater, output *.**
			PUSH{R0,LR}
			MOV R0, #0x2A
			BL ST7735_OutChar
			MOV R0, #0x2E
			BL ST7735_OutChar
			MOV R0, #0x2A
			BL ST7735_OutChar
			MOV R0, #0x2A
			BL ST7735_OutChar
			POP{R0,LR}
			B doneOF2
	
doneOF2      BX  LR
     
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
