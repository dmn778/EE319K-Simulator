# EE319K-Simulator
Created for EE319K Game Competition for the Fall 2019 semester. This game placed 3rd.

The game was written in C and played on an ARM-Cortex TM4C Microcontroller.

INPUTS:
  * Joystick
  * Two buttons
  
  
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
