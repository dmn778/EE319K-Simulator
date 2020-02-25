# EE319K-Simulator
This bullet hell themed around EE319K was created for EE319K Game Competition for the Fall 2019 semester. This game placed 3rd.

Objective: You are an average student in EE319K. Shoot at the labs to get them done but avoid getting shot by bugs/bad test scores. If you grade drops to 0, you fail the class. If you beat all the labs you pass!

Controls: Use the joystick to move your character and dodge attacks. Careful! If you get hit too much your grade will drop to 0 and you will fail the class. Use button 1 to shoot back at the labs to get them done. Button 2 is a special powerup that temporarily clears all the attacks on the screen.

The game was written in C and played on an ARM-Cortex TM4C Microcontroller.
The game's main source code is written in Lab8.c

INPUTS:
  * Joystick
    ; X connected to PD2
    ; Y connected to PD3
    ; Btn unused
  * Two buttons (negative logic)
    ; Shoot button connected to PE1
    ; Power-up button connected to PE2
  
OUTPUTS:
  * ST7735 1.8″ COLOR TFT DISPLAY
    ; Backlight (pin 10) connected to +3.3 V
    ; MISO (pin 9) unconnected
    ; SCK (pin 8) connected to PA2 (SSI0Clk)
    ; MOSI (pin 7) connected to PA5 (SSI0Tx)
    ; TFT_CS (pin 6) connected to PA3 (SSI0Fss)
    ; CARD_CS (pin 5) unconnected
    ; Data/Command (pin 4) connected to PA6 (GPIO)
    ; RESET (pin 3) connected to PA7 (GPIO)
    ; VCC (pin 2) connected to +3.3 V
    ; Gnd (pin 1) connected to ground
 * (OPTIONAL) DAC for sound output
    ; PB0-PB5
