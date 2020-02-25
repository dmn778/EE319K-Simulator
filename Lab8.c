// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: Neil Narvekar, Dat Nguyen
// Last modification date: 11/10/2019
// Last Modified: 9/2/2019 

// Specifications:
// Measure distance using slide pot, sample at 60 Hz
// maximum distance can be any value from 1.5 to 2cm
// minimum distance is 0 cm
// Calculate distance in fixed point, 0.01cm
// Analog Input connected to PD2=ADC5
// displays distance on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats (use them in creative ways)
// 

#include <stdint.h>
#include "ST7735.h"
//#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "print.h"
#include "../inc/tm4c123gh6pm.h"
#include "Random.h"
#include "PLL.h"
#include "IO.h"

typedef enum {English, Spanish} Language_t;
Language_t myLanguage = English;
typedef enum {WELCOME, TITLE, START, LANGUAGES, LANGUAGE, GRADE, BOSS, LOSE, WIN, SCORE, RESET, RESET2, TAUNT1_1, TAUNT1_2, RESPONSE1, TAUNT2_1, TAUNT2_2, RESPONSE2} phrase_t;
const char Welcome_English[] = "Welcome to";
const char Welcome_Spanish[] = "Bienvenido a";
const char Title_English[] = "EE319K Simulator";
const char Title_Spanish[] = "EE319K Simulador";
const char Start_English[]= "Start";
const char Start_Spanish[]= "Empezar";
const char Languages_English[] = "Languages";
const char Languages_Spanish[] = "Idiomas";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Grade_English[]="Your Grade:";
const char Grade_Spanish[]="Tu Nota:";
const char Boss_English[]="Boss Health:";
const char Boss_Spanish[]="Salud de Jefe:";
const char Lose_English[]="You Failed!";
const char Lose_Spanish[]="\xADT\xA3 Fallaste!";
const char Win_English[]="You Passed!";
const char Win_Spanish[]="\xADT\xA3 Pasaste!";
const char Score_English[]="Score:";
const char Score_Spanish[]="Puntuaci\xA2n:";
const char Reset_English[]="Press Reset";
const char Reset_Spanish[]="Presiona Reiniciar";
const char Reset2_English[]="to play again.";
const char Reset2_Spanish[]="para jugar otra vez.";
const char Taunt1_1_English[]="I see you";
const char Taunt1_1_Spanish[]="Te veo que";
const char Taunt1_2_English[]="passed my labs.";
const char Taunt1_2_Spanish[]="pasaste mis tareas.";
const char Response1_English[]=">Barely.";
const char Response1_Spanish[]=">Apenas.";
const char Taunt2_1_English[]="Did you think it";
const char Taunt2_1_Spanish[]="Crees que";
const char Taunt2_2_English[]="would be that easy?";
const char Taunt2_2_Spanish[]="sería tan fácil?";
const char Response2_English[]=">Wait, I didn't say that.";
const char Response2_Spanish[]=">Apenas.";

const char *Phrases[15][2]={
	{Welcome_English, Welcome_Spanish},
  {Title_English, Title_Spanish},
  {Start_English, Start_Spanish},
	{Languages_English, Languages_Spanish},
  {Language_English, Language_Spanish},
	{Grade_English, Grade_Spanish},
	{Boss_English, Boss_Spanish},
	{Lose_English, Lose_Spanish},
	{Win_English, Win_Spanish},
	{Score_English, Score_Spanish},
	{Reset_English, Reset_Spanish},
	{Reset2_English, Reset2_Spanish},
	{Taunt1_1_English, Taunt1_1_Spanish},
	{Taunt1_2_English, Taunt1_2_Spanish},
	{Response1_English, Response1_Spanish},
};

const uint8_t duty_size = 4;
uint8_t duty_counter = 0;
const uint8_t duty_cycle[duty_size] ={
	1,2,3,4
};

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Timer0A_Init(uint32_t period);
void Timer1A_Init(uint32_t period);
uint32_t xADC_In(void);
uint32_t yADC_In(void);
void SysTick_Init(void);
void PortF_Init(void);
void PortE_Init(void);
uint32_t button(void);
void DAC_Out(uint32_t data);
void ClearAttacks(void);
void ST7735_OutPhrase(phrase_t message);
void DrawPowerUp(void);

//********************************************************************************** 
// GAME STARTS HERE

// GLOBAL VARIABLES
typedef enum {CIRCLE, BEAM, SPRAY, RSPRAY, NONE, ALL} attack_t;
attack_t last_attack = NONE;
attack_t chosen_attack = NONE;

uint8_t piazza_counter = 2;
uint32_t test_data;
uint32_t Data;        // 12-bit ADC
int32_t pos;    // 32-bit fixed-point 0.001 cm
int32_t xPosition, yPosition;
uint8_t piazzaflag = 10;
uint8_t hurt_sound = 0;
uint8_t piazza_sound = 0;
uint8_t shoot_sound = 0;
uint8_t valvano_sound = 0;
uint8_t check_end_flag = 1;
uint8_t click_sound = 0;
uint32_t index_hurt=0;
uint32_t index_piazza=0;
uint32_t index_shoot=0;
uint32_t Index = 0;
uint32_t index_click = 0;

// game status
// 0 is menu
// -1 is languages
// 1, 2, 3 is 1st, 2nd, 3rd boss
// 4 is lose screen
// 5 is win screen
// 6 is between boss 1 and 2
// 7 is between boss 2 and 3
// 8 is tutorial 1
// 9 is tutorial 2
// 10 is tutorial 3
int8_t game_status = 0;

// flags to track if player has killed bosses or not,
// set to 1 if player has defeated boss
uint8_t boss1_dead = 0;
uint8_t boss2_dead = 0;
uint8_t boss3_dead = 0;
uint8_t wave_wait = 1;
uint8_t beam_wait = 1;
uint32_t attack_cooldown = 60;
uint32_t attack_counter = 61;
uint8_t spray_wait = 1;
uint8_t sprayattackflag = 0;
uint8_t rand_spray_wait = 1;
uint8_t randsprayattackflag = 0;
uint8_t circle_wait = 1;
uint8_t circleattackflag = 0;

// 0 is top option
// 1 is bottom option
int8_t cursor = 0;

int32_t ADCStatus=0; // 1 means new data available
int32_t xADCMail,yADCMail;

// struct to hold sprite data
struct State{
	int32_t x; // x position
	int32_t y; // y position
	const unsigned short *image; // ptr to image
	const unsigned short *dead; // ptr to image
	const int32_t width; // width
	const int32_t height; // height
	int32_t xvelocity;
	int32_t yvelocity;
	uint8_t moving;
	int8_t health;
};

struct State_float{
	float x; // x position
	float y; // y position
	const unsigned short *image; // ptr to image
	const unsigned short *dead; // ptr to image
	const int32_t width; // width
	const int32_t height; // height
	float xvelocity;
	float yvelocity;
	uint8_t moving;
	int8_t health;
};

typedef struct State StateType;
//sprite[0] stores player
//sprite[1] stores boss2
//sprite[2] stores boss1
//sprite[3] stores boss3
//sprite[4] stores valvano_large
StateType sprite[5] = {
	{60,120, naruto,0, 14,16,0,0,0,100}, // Player s`prite
	{60, 32, guitar_small, guitar_dead, guitarwidth,guitarheight,-1,0,0,15}, // Boss Sprite
	{50, 32, LED, LED_dead, LEDwidth, LEDheight, 1,0,0,10}, // LED 15
	{50, 50, valvano_small, valvano_dead, valvano_small_width, valvano_small_height, 1,0,0,15}, //valvano
	{9, 110, valvano_large, valvano_dead, valvano_large_width, valvano_large_height, 1,0,0,10}
};

const uint32_t PlayerAttackStructSize = 15;
// MAX AMOUNT OF PLAYER ATTACKS = PlayerAttackStructSize
typedef struct State StateType;
StateType PlayerAttack[PlayerAttackStructSize] = {
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 1
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 2
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack ... etc.
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack     
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 
	{0, 0, playerattack,deadplayerattack,playerattackwidth,playerattackheight,0,-1,0,0}, // Player attack 
};

const uint32_t StraightAttackStructSize = 8;
typedef struct State StateType;
StateType BossAttack[StraightAttackStructSize] = {
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, // straight attacks
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},    
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, // straight attacks
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},
	{0, 0, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},	
};

const uint32_t WaveAttackStructSize = 10;
typedef struct State StateType;
StateType WaveAttackStruct[WaveAttackStructSize] = {
	{32, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, // WAVE attacks
	{28, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{24, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},
	{28, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0},
	{32, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{36, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{40, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{36, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{32, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
	{28, 32, bossattack,blackbossattack, bossattackwidth,bossattackheight,0,1,0,0}, 
};

const uint32_t BeamAttackStructSize = 24;
typedef struct State StateType;
StateType BeamAttackStruct[BeamAttackStructSize] = {
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
	{0, 0, LEDpurple,LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,1,0,0},
};

const uint32_t CircleAttackStructSize = 27;
typedef struct State_float StateType_float;
StateType_float CircleAttackStruct[CircleAttackStructSize] = {
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-2,0,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,0,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-2,0,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,0,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-2,0,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0.765,1.85,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.414,1.414,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1.85,0.765,0,0},
	{0, 0, LEDred, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,0,0,0},		
};

const uint32_t SprayAttackStructSize = 25;
typedef struct State StateType;
StateType SprayAttackStruct[SprayAttackStructSize] = {
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-4,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,-3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,-2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,4,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-4,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,-1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,1,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,2,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,3,2,0,0},
	{0, 0, LEDblue, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,4,2,0,0},
};

const uint32_t RandSprayAttackStructSize = 20;
typedef struct State StateType;
StateType RandSprayAttackStruct[RandSprayAttackStructSize] = {
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth, LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
	{0, 0, LEDgreen, LEDdeadpurple, LEDpurplewidth,LEDpurplewidth,0,3,0,0},
};

const uint32_t ShineAttackStructSize = 36;
typedef struct State StateType;
StateType ShineAttackStruct[ShineAttackStructSize] = {
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,-1,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,0,1,0,0},
	{0, 0, LEDattack, LEDdeadattack, LEDattackwidth,LEDattackheight,1,1,0,0},
};

const uint32_t PiazzaStructSize = 2;
typedef struct State StateType;
StateType PiazzaStruct[PiazzaStructSize] = {
	{95, 10, piazza_small,piazza_small_dead, 12,12,0,0,1}, 
	{95, 10, piazza_small,piazza_small_dead, 12,12,0,1,1},
};


//check collision method
//returns 1 if collision between the two sprites is detected
//0 otherwise
int CheckCollision(struct State *rect1, struct State *rect2){
	int32_t r1_x = rect1->x + 2; //to account for 2 pixel wide black rectangle on each side of sprite
	int32_t r1_y = rect1->y + 1; // 1 pixel wide rectangle above and below
	int32_t r2_x = rect2->x + 2;
	int32_t r2_y = rect2->y + 1;
	uint32_t r1_width = rect1->width - 4;
	uint32_t r1_height = rect1->height - 2;
	uint32_t r2_width = rect2->width - 4;
	uint32_t r2_height = rect2->height - 2;
	if (r1_x <= r2_x + r2_width &&
   r1_x + r1_width >= r2_x&&
   r1_y - r1_height <= r2_y &&
   r1_y >= r2_y - r2_height) {
    // collision detected!
		 return 1;
	}
	 return 0;
}

int CheckCollisionNoOffset(struct State *rect1, struct State *rect2){
	int32_t r1_x = rect1->x;
	int32_t r1_y = rect1->y; 
	int32_t r2_x = rect2->x;
	int32_t r2_y = rect2->y;
	uint32_t r1_width = rect1->width;
	uint32_t r1_height = rect1->height;
	uint32_t r2_width = rect2->width;
	uint32_t r2_height = rect2->height;
	if (r1_x <= r2_x + r2_width &&
   r1_x + r1_width >= r2_x&&
   r1_y - r1_height <= r2_y &&
   r1_y >= r2_y - r2_height) {
    // collision detected!
		 return 1;
	}
	 return 0;
}

int CheckCollisionNoOffsetFloat(struct State *rect1, struct State_float *rect2){
	int32_t r1_x = rect1->x;
	int32_t r1_y = rect1->y;
	int32_t r2_x = rect2->x;
	int32_t r2_y = rect2->y;
	uint32_t r1_width = rect1->width;
	uint32_t r1_height = rect1->height;
	uint32_t r2_width = rect2->width;
	uint32_t r2_height = rect2->height;
	if (r1_x <= r2_x + r2_width &&
   r1_x + r1_width >= r2_x&&
   r1_y - r1_height <= r2_y &&
   r1_y >= r2_y - r2_height) {
    // collision detected!
		 return 1;
	}
	 return 0;
}

//generate random attack for boss 3

uint32_t valvano_chance = 5; //set higher for higher chance for valvano's ultimate attack
// MAX IS 63 DO NO SET ANY HIGHER
void generateRandomAttack(){
do{
	uint32_t rand = Random();
	if(sprite[3].health < 8){

  if(rand < 63 - valvano_chance){
		chosen_attack = CIRCLE;
	}
	else if(rand < 127 - 2*valvano_chance){
		chosen_attack = SPRAY;
	}
	else if(rand < 191 - 3*valvano_chance ){
		chosen_attack = RSPRAY;
	}
	else if(rand < 256 - 4*valvano_chance){
		chosen_attack = BEAM;
	}
	else{ //rare chance for valvano to use all his attacks at once
		chosen_attack = ALL;
		valvano_sound = 1;
		Index = 0;
	}
}
	else{
	 if(rand < 63){
		chosen_attack = CIRCLE;
	}
	else if(rand < 127){
		chosen_attack = SPRAY;
	}
	else if(rand < 191 ){
		chosen_attack = RSPRAY;
	}
	else{
		chosen_attack = BEAM;
	}
	}
}while(chosen_attack == last_attack);
last_attack = chosen_attack;
}

uint32_t Convert(uint32_t input){
	// converts ADC value into a position output
	// starts 2000, right max = 4000, left max = 0. up max = 0, down max = 4000
	pos = ((input+400)/2000) - 1; 
  return pos; 
}

// ------- xRandom and yRandom Convert Random number generated into useable number
uint32_t xRandom(void){
	// x goes from 0-128, Random() goes from 0 to 255
	uint32_t ran = Random()*128/255;
	return ran;
}
uint32_t yRandom(void){
	// y goes from 0-160, Random() goes from 0 to 255
	uint32_t ran = Random()*160/255;
	return ran;
}
uint32_t BinaryRandom(void){
	// y goes from 0-160, Random() goes from 0 to 1
	uint32_t ran = Random()/(255/2);
	return ran;
}

//returns an random integer from -2 to 2
int32_t Rand_2from0(void){
	uint32_t rand = Random();
	if(rand < 51){
		return -2;
	}
	else if(rand < 102){
		return -1;
	}
	else if(rand < 153){
		return 0;
	}
	else if(rand < 204){
		return 1;
	}
	return 2;
}

int32_t Rand_1from0(void){
	uint32_t rand = Random();
	if(rand < 88){
		return -1;
	}
	else if(rand < 176){
		return 0;
	}
	return 1;
}

//--- SYSTICK HANDLER SAMPLES THE ADC FOR PLAYER MOVEMENT
int32_t PlayerAttackFlag=0;
void SysTick_Handler(void){
	// SysTick_Handler samples the ADC
	IO_HeartBeat();
	xADCMail = yADC_In();
	yADCMail = xADC_In();
	ADCStatus = 1;
	
	if(button() == 1 && game_status > 0){
		PlayerAttackFlag++;
	}
}
void PlayerTakeDamage(void){
	if(sprite[0].health > 0){
		sprite[0].health -=5;
		hurt_sound = 1;
		index_hurt = 0;
	}
}

//--- TIMER0A_HANDLER MAKES THE BOSS MOVE
uint8_t straightattackflag = 0;
uint8_t shineattackflag = 0;
uint8_t pulseattackflag = 0;
uint8_t waveattackflag = 0;
uint8_t tracerattackflag = 0;
uint8_t beamattackflag = 0;
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;  // acknowledge timer0A timeout
	if(game_status == 1){
		shineattackflag++;
		pulseattackflag++;
	}
	else if(game_status == 2){
	  straightattackflag++;
	  waveattackflag++;
	//tracerattackflag++;
	}
}

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


// TIMER1A_Handler makes the sound happen
void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;  // acknowledge timer0A timeout
	if(valvano_sound == 1){
		DAC_Out(valvano[Index]);
	  Index++;
		if(Index >= valvano_length){
			valvano_sound = 0;
			Index = 0;
		}
	}
	else if(hurt_sound == 1){
		DAC_Out(oof[index_hurt]);
	  index_hurt++;
		if(index_hurt >= oof_length){
			index_hurt = 0;
			hurt_sound = 0;
		}
	}
	else if(piazza_sound == 1){
		DAC_Out(power_up[index_piazza]);
	  index_piazza++;
		if(index_piazza >= power_up_length){
			index_piazza = 0;
			piazza_sound = 0;
		}
	}
	else if(shoot_sound == 1){
		DAC_Out(pew[index_shoot]);
	  index_shoot++;
		if(index_shoot >= pew_length){
			index_shoot = 0;
			shoot_sound = 0;
		}
	}
	else if(click_sound == 1){
		DAC_Out(click[index_click]);
	  index_click++;
		if(index_click >= click_length){
			index_click = 0;
			click_sound = 0;
		}
	}
}


// -------Draw Player Sprite Function
void DrawPlayer(void){
	uint32_t i,speed=3;
	for(i=0;i<speed;i++){
		if((sprite[0].x - xPosition) < 0 || (sprite[0].x - xPosition +sprite[0].width - 2 > 128) || ((sprite[0].y - yPosition - sprite[0].height) < 16) || (sprite[0].y - yPosition > 160)){
			break; //use yposition <16 if you dont want player to go into boss health text
		}
		sprite[0].x = sprite[0].x - xPosition;
		sprite[0].y = sprite[0].y - yPosition; // subtract because y is flipped on joystick for some reason
		ST7735_DrawBitmap(sprite[0].x, sprite[0].y, sprite[0].image, sprite[0].width,sprite[0].height);	
		if((game_status == 2) && (CheckCollision(&sprite[1], &sprite[0]) == 1)){
			PlayerTakeDamage();
		}
		else if((game_status == 1) && (CheckCollision(&sprite[2], &sprite[0]) == 1)){
			PlayerTakeDamage();
		}
		else if((game_status == 3) && (CheckCollision(&sprite[3], &sprite[0]) == 1)){
			PlayerTakeDamage();
		}
	}
}

uint32_t speed_boss_2 = 1;
int32_t oldx,oldy = 0;
//-----Draw Boss Sprite Function
void DrawBoss2(void){
	uint32_t i;
	if(Random() == 1){
		sprite[1].xvelocity = -1;
	}
	if(Random() == 254){
		sprite[1].xvelocity = 1;
	}
	oldx = sprite[1].x;
	oldy = sprite[1].y;
	for(i=0;i<speed_boss_2;i++){
		if(((sprite[1].x + sprite[1].xvelocity) < 15  || (sprite[1].x +  sprite[1].xvelocity +sprite[1].width > 110))){
			sprite[1].xvelocity *= -1;
		}
		sprite[1].x = sprite[1].x + sprite[1].xvelocity;
		sprite[1].y = sprite[1].y + sprite[1].yvelocity;	
	}
	ST7735_FillRect(oldx, oldy, sprite[1].width, sprite[1].height, ST7735_BLACK);
	ST7735_DrawBitmap(sprite[1].x, sprite[1].y, sprite[1].image, sprite[1].width, sprite[1].height);
	//sprite[1].xvelocity = 0;
}
uint32_t speed_boss_3 = 2;
void DrawBoss3(void){
	uint32_t i;
	if(Random() <50 ){
		sprite[3].xvelocity *= -1;
	}
	oldx = sprite[3].x;
	oldy = sprite[3].y;
	for(i=0;i<speed_boss_3;i++){
		if(((sprite[3].x + sprite[3].xvelocity) < 12  || (sprite[3].x +  sprite[3].xvelocity +sprite[3].width > 120))){
			sprite[3].xvelocity *= -1;
		}
		sprite[3].x = sprite[3].x + sprite[3].xvelocity;
		sprite[3].y = sprite[3].y + sprite[3].yvelocity;	
	}
	//ST7735_FillRect(oldx, oldy, sprite[1].width, sprite[1].height, ST7735_BLACK);
	ST7735_DrawBitmap(sprite[3].x, sprite[3].y, sprite[3].image, sprite[3].width, sprite[3].height);
}

// randomly initializes all bosses direction
void BossInit(void){
	//boss 2
	if(Random() < 127){
		sprite[1].xvelocity = -1;
	}
	else{
		sprite[1].xvelocity = 1;
	}
	//boss 1
	if(Random() < 127){
		sprite[2].xvelocity = -1;
	}
	else{
		sprite[2].xvelocity = 1;
	}
	//boss 3
	if(Random() < 127){
		sprite[3].xvelocity = -1;
	}
	else{
		sprite[3].xvelocity = 1;
	}
}

uint32_t speed_boss_1 = 1;
//-----Draw Boss Sprite Function
void DrawBoss1(void){
	uint32_t i;
	oldx = sprite[2].x;
	oldy = sprite[2].y;
	for(i=0;i<speed_boss_1;i++){
		if(((sprite[2].x + sprite[2].xvelocity) < 15  || (sprite[2].x +  sprite[2].xvelocity +sprite[2].width > 110))){
			sprite[2].xvelocity *= -1;
		}
		sprite[2].x = sprite[2].x + sprite[2].xvelocity;
		sprite[2].y = sprite[2].y + sprite[2].yvelocity;	
	}
	ST7735_FillRect(oldx, oldy, sprite[2].width, sprite[2].height, ST7735_BLACK);
	ST7735_DrawBitmap(sprite[2].x, sprite[2].y, sprite[2].image, sprite[2].width, sprite[2].height);
}

uint8_t speed_piazza = 1;
void DrawPowerUp(void){
	for(uint8_t j=0; j<speed_piazza; j++){
			if(PiazzaStruct[1].moving == 1){
				PiazzaStruct[1].y = PiazzaStruct[1].y + PiazzaStruct[1].yvelocity; 
				ST7735_DrawBitmap(PiazzaStruct[1].x, PiazzaStruct[1].y, PiazzaStruct[1].image, PiazzaStruct[1].width, PiazzaStruct[1].height);	
				if(CheckCollision(&sprite[0], &PiazzaStruct[1]) == 1){ //if player collects power up
					PiazzaStruct[1].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(PiazzaStruct[1].x, PiazzaStruct[1].y, PiazzaStruct[1].dead, PiazzaStruct[1].width,PiazzaStruct[1].height);	// when collides, disappear
				  piazza_counter++;
					if(game_status == 6){
						game_status = 2;
					}
					else if(game_status == 7){
						valvano_sound = 1;
						game_status = 8;
						ST7735_FillScreen(ST7735_BLACK);
					}
				}
				if(PiazzaStruct[1].y > 160){
					PiazzaStruct[1].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(PiazzaStruct[1].x, PiazzaStruct[1].y, PiazzaStruct[1].dead, PiazzaStruct[1].width,PiazzaStruct[1].height);	// when collides, disappear
				  if(game_status == 6){
						game_status = 2;
					}
					else if(game_status == 7){
						valvano_sound = 1;
						game_status = 8;
						ST7735_FillScreen(ST7735_BLACK);
					}
				}
			}
		}
}


//--------Draw Player Attack Function
uint32_t pa=0;
void DrawPlayerAttack(void){
	// Draws an attack to fly across screen
	// Arguement is which attack it is in the array
	uint32_t i=0,j,speed=4;
	
	if(PlayerAttackFlag > 12){ // check to see button held long enough
		if(PlayerAttack[pa].moving == 0){ // if not moving already
			PlayerAttack[pa].x = sprite[0].x+4; // put attack where player is
			PlayerAttack[pa].y = sprite[0].y-sprite[0].height; 
			ST7735_DrawBitmap(PlayerAttack[pa].x, PlayerAttack[pa].y, PlayerAttack[pa].image, PlayerAttack[pa].width,PlayerAttack[pa].height);
			PlayerAttack[pa].moving = 1; // set the attack to moving
			shoot_sound = 1;
			index_shoot = 0;
			pa++; // increment the attacks
			PlayerAttackFlag = 0;
		}
	}
	for(i=0;i<PlayerAttackStructSize;i++){
		for(j=0;j<speed;j++){
			if(PlayerAttack[i].moving == 1){
				PlayerAttack[i].x = PlayerAttack[i].x + PlayerAttack[i].xvelocity; // move the attack
				PlayerAttack[i].y = PlayerAttack[i].y + PlayerAttack[i].yvelocity; 
				ST7735_DrawBitmap(PlayerAttack[i].x, PlayerAttack[i].y, PlayerAttack[i].image, PlayerAttack[i].width,PlayerAttack[i].height);	
				
				if((game_status == 2) && (CheckCollision(&sprite[1], &PlayerAttack[i]) == 1)){
					PlayerAttack[i].moving = 0; // the attack collides with the boss sprite, make it not move anymore
					ST7735_DrawBitmap(PlayerAttack[i].x, PlayerAttack[i].y, PlayerAttack[i].dead, PlayerAttack[i].width,PlayerAttack[i].height);
					sprite[1].health--;		
				}
				else if((game_status == 1) && (CheckCollision(&sprite[2], &PlayerAttack[i]) == 1)){
					PlayerAttack[i].moving = 0; // the attack collides with the boss sprite, make it not move anymore
					ST7735_DrawBitmap(PlayerAttack[i].x, PlayerAttack[i].y, PlayerAttack[i].dead, PlayerAttack[i].width,PlayerAttack[i].height);
					sprite[2].health--;		
				}
				else if((game_status == 3) && (CheckCollision(&sprite[3], &PlayerAttack[i]) == 1)){
					PlayerAttack[i].moving = 0; // the attack collides with the boss sprite, make it not move anymore
					ST7735_DrawBitmap(PlayerAttack[i].x, PlayerAttack[i].y, PlayerAttack[i].dead, PlayerAttack[i].width,PlayerAttack[i].height);
					sprite[3].health--;		
				}
				if((PlayerAttack[i].x < 0) || (PlayerAttack[i].x > 128) || (PlayerAttack[i].y < 25) || (PlayerAttack[i].y > 160) ){
					PlayerAttack[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(PlayerAttack[i].x, PlayerAttack[i].y, PlayerAttack[i].dead, PlayerAttack[i].width,PlayerAttack[i].height);
				}
			}
		}
	}
	if(pa >= PlayerAttackStructSize){ // game can only hold a number of attacks equal to PlayerAttackStructSize
		pa = 0;
	}
}


// ---------------  BOSS ATTACKS START HERE -----------------
// --------- Straight shot attack
uint32_t straighti=0;
void StraightAttack(void){
	uint32_t i,j,speed=2;
	if(straightattackflag >= 2){ // use flag to time attacks
		if(BossAttack[straighti].moving == 0){ // if not moving already
				BossAttack[straighti].x = xRandom(); // place where attack starts
				BossAttack[straighti].y = sprite[1].y + sprite[1].height; 
				ST7735_DrawBitmap(BossAttack[straighti].x, BossAttack[straighti].y, BossAttack[straighti].image, BossAttack[straighti].width,BossAttack[straighti].height);
				BossAttack[straighti].moving = 1; // set the attack to moving
				straighti++;
				straightattackflag = 0;
		}
	}
	for(i=0;i<StraightAttackStructSize;i++){ // move attacks one by one
		for(j=0;j<speed;j++){
			if(BossAttack[i].moving == 1){
				BossAttack[i].x = BossAttack[i].x + BossAttack[i].xvelocity; // move the attack
				BossAttack[i].y = BossAttack[i].y + BossAttack[i].yvelocity; 
				ST7735_DrawBitmap(BossAttack[i].x, BossAttack[i].y, BossAttack[i].image, BossAttack[i].width,BossAttack[i].height);	
				if(CheckCollision(&sprite[0], &BossAttack[i]) == 1){
					BossAttack[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(BossAttack[i].x, BossAttack[i].y, BossAttack[i].dead, BossAttack[i].width,BossAttack[i].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				if((BossAttack[i].x < 0) || (BossAttack[i].x > 128) || (BossAttack[i].y < 0) || (BossAttack[i].y > 160) ){
					BossAttack[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(BossAttack[i].x, BossAttack[i].y, BossAttack[i].dead, BossAttack[i].width,BossAttack[i].height);	// when collides, disappear
				}
			}
		}
	}
	if(straighti >= StraightAttackStructSize){ // game can only hold a number of attacks equal to StraightAttackStructSize
		straighti = 0;
	}
}
//---- SHINE ATTACK
uint32_t shinei=0;
uint32_t rollover=0;
uint32_t shineshots=0;

void incrementShinei(void){
	rollover = ShineAttackStructSize - 9*duty_counter;
	shineshots ++;
	shinei = ((shinei+1)%ShineAttackStructSize);
	if(shineshots >= rollover){
		duty_counter = (duty_counter+1) % duty_size;
		shineattackflag = 0;
		shineshots = 0;
	}
}
void ShineAttack(void){
	int32_t si,j;
	uint32_t speed=2;
	if(shineattackflag >= duty_cycle[duty_counter]){ // use flag to time attacks
		if(ShineAttackStruct[shinei].moving == 0){ // if not moving already
				ShineAttackStruct[shinei].x = sprite[2].x + sprite[2].width - 8; // place where attack starts
				ShineAttackStruct[shinei].y = sprite[2].y + 4; 
				ST7735_DrawBitmap(ShineAttackStruct[shinei].x, ShineAttackStruct[shinei].y, ShineAttackStruct[shinei].image, ShineAttackStruct[shinei].width,ShineAttackStruct[shinei].height);
				ShineAttackStruct[shinei].moving = 1; // set the attack to moving
				incrementShinei();
				ShineAttackStruct[shinei].x = sprite[2].x + sprite[2].width - 8; // place where attack starts
				ShineAttackStruct[shinei].y = sprite[2].y + 4; 
				ST7735_DrawBitmap(ShineAttackStruct[shinei].x, ShineAttackStruct[shinei].y, ShineAttackStruct[shinei].image, ShineAttackStruct[shinei].width,ShineAttackStruct[shinei].height);
				ShineAttackStruct[shinei].moving = 1; // set the attack to moving
				incrementShinei();				
			  ShineAttackStruct[shinei].x = sprite[2].x + sprite[2].width - 8; // place where attack starts
				ShineAttackStruct[shinei].y = sprite[2].y + 4; 
				ST7735_DrawBitmap(ShineAttackStruct[shinei].x, ShineAttackStruct[shinei].y, ShineAttackStruct[shinei].image, ShineAttackStruct[shinei].width,ShineAttackStruct[shinei].height);
				ShineAttackStruct[shinei].moving = 1; // set the attack to moving
				incrementShinei();
				shineattackflag = 0;
		}
	}
	for(si=ShineAttackStructSize-1; si>=0; si--){ // move attacks one by one
		for(j=0;j<speed;j++){
			if(ShineAttackStruct[si].moving == 1){
				ShineAttackStruct[si].x = ShineAttackStruct[si].x + ShineAttackStruct[si].xvelocity; // move the attack
				ShineAttackStruct[si].y = ShineAttackStruct[si].y + ShineAttackStruct[si].yvelocity; 
				ST7735_DrawBitmap(ShineAttackStruct[si].x, ShineAttackStruct[si].y, ShineAttackStruct[si].image, ShineAttackStruct[si].width,ShineAttackStruct[si].height);	
				if(CheckCollision(&sprite[0], &ShineAttackStruct[si]) == 1){
					ShineAttackStruct[si].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(ShineAttackStruct[si].x, ShineAttackStruct[si].y, ShineAttackStruct[si].dead, ShineAttackStruct[si].width,ShineAttackStruct[si].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				if((ShineAttackStruct[si].x < 0) || (ShineAttackStruct[si].x > 128) || (ShineAttackStruct[si].y < 0) || (ShineAttackStruct[si].y > 154) ){
					ShineAttackStruct[si].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(ShineAttackStruct[si].x, ShineAttackStruct[si].y, ShineAttackStruct[si].dead, ShineAttackStruct[si].width,ShineAttackStruct[si].height);	// when collides, disappear
				}
			}
		}
	}
}

uint8_t circlei = 0;

//helper function to check if circles are all gone from screen
//returns 0 if one is still moving, 1 if all are not moving
uint8_t CircleAllGone(void){
	for(int x = 0; x<CircleAttackStructSize; x++){
		if(CircleAttackStruct[x].moving == 1){
			return 0;
		}
	}
	return 1;
}
void CircleAttack(void){
	int32_t si,j;
	uint32_t speed=1;
	if(circleattackflag >= 6 && circle_wait == 1 && (chosen_attack == CIRCLE || chosen_attack == ALL)){ // use flag to time attacks
		for(int x = 0; x < 9; x++){
		if(CircleAttackStruct[circlei].moving == 0){ // if not moving already
				CircleAttackStruct[circlei].x = sprite[3].x + 10; // place where attack starts
				CircleAttackStruct[circlei].y = sprite[3].y; 
				ST7735_DrawBitmap(CircleAttackStruct[circlei].x, CircleAttackStruct[circlei].y, CircleAttackStruct[circlei].image, CircleAttackStruct[circlei].width,CircleAttackStruct[circlei].height);
				CircleAttackStruct[circlei].moving = 1; // set the attack to moving
				circlei++;
		}
		}
		circleattackflag = 0;
	}
	for(si=CircleAttackStructSize-1; si>=0; si--){ // move attacks one by one
		for(j=0;j<speed;j++){
			if(CircleAttackStruct[si].moving == 1){
				ST7735_DrawBitmap(CircleAttackStruct[si].x, CircleAttackStruct[si].y, CircleAttackStruct[si].dead, CircleAttackStruct[si].width,CircleAttackStruct[si].height);	
				CircleAttackStruct[si].x = CircleAttackStruct[si].x + CircleAttackStruct[si].xvelocity; // move the attack
				CircleAttackStruct[si].y = CircleAttackStruct[si].y + CircleAttackStruct[si].yvelocity; 
				ST7735_DrawBitmap(CircleAttackStruct[si].x, CircleAttackStruct[si].y, CircleAttackStruct[si].image, CircleAttackStruct[si].width,CircleAttackStruct[si].height);	
				if(CheckCollisionNoOffsetFloat(&sprite[0], &CircleAttackStruct[si]) == 1){
					CircleAttackStruct[si].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(CircleAttackStruct[si].x, CircleAttackStruct[si].y, CircleAttackStruct[si].dead, CircleAttackStruct[si].width,CircleAttackStruct[si].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				else if((CircleAttackStruct[si].x < 0) || (CircleAttackStruct[si].x > 128) || (CircleAttackStruct[si].y < 0) || (CircleAttackStruct[si].y > 154) ){
					CircleAttackStruct[si].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(CircleAttackStruct[si].x, CircleAttackStruct[si].y, CircleAttackStruct[si].dead, CircleAttackStruct[si].width,CircleAttackStruct[si].height);	// when collides, disappear
				}
			}
		}
	}
	if((circlei >= CircleAttackStructSize) && CircleAllGone() == 1){ // game can only hold a number of attacks equal to StraightAttackStructSize
		circlei = 0;
		circle_wait = 1;
	}
	else if(circlei >= CircleAttackStructSize){
		circle_wait = 0;
	}
}

// --------- WAVE SHOT ATTACK----------------
int32_t waveinitx[WaveAttackStructSize] = {30,26,22,26,30,34,38,34,30,26};
int32_t waveinity[WaveAttackStructSize] = {32,32,32,32,32,32,32,32,32,32};
uint32_t wavei=0;
void WaveAttackInit(void){
	wavei=0;
	uint32_t i,x,y,xstart;
	x = xRandom();
	y = yRandom();
	while(y < 44){
		y =yRandom();
	}
	while((x > 116) || (x < 10)){
		x = xRandom();
	}
	if(BinaryRandom() == 1){ // THIS SHOOTS FROM TOP
		for(i=0;i<WaveAttackStructSize;i++){
			WaveAttackStruct[i].y = 44;
			WaveAttackStruct[i].xvelocity = 0;
			WaveAttackStruct[i].yvelocity = 1;
		}
		WaveAttackStruct[0].x = x;
		WaveAttackStruct[1].x = x-4;
		WaveAttackStruct[2].x = x-8;
		WaveAttackStruct[3].x = x-4;
		WaveAttackStruct[4].x = x;
		WaveAttackStruct[5].x = x+4;
		WaveAttackStruct[6].x = x+8;
		WaveAttackStruct[7].x = x+4;
		WaveAttackStruct[8].x = x;
		WaveAttackStruct[9].x = x-4;
	}else{ // THIS SHOOTS FROM SIDE
		if(BinaryRandom() == 1){
			xstart = 0;
		}else{
		xstart = 160;
		}
		for(i=0;i<WaveAttackStructSize;i++){
			WaveAttackStruct[i].x = xstart;
			WaveAttackStruct[i].xvelocity = 1;
			WaveAttackStruct[i].yvelocity = 0;
		}
		WaveAttackStruct[0].y = y;
		WaveAttackStruct[1].y = y-4;
		WaveAttackStruct[2].y = y-8;
		WaveAttackStruct[3].y = y-4;
		WaveAttackStruct[4].y = y;
		WaveAttackStruct[5].y = y+4;
		WaveAttackStruct[6].y = y+8;
		WaveAttackStruct[7].y = y+4;
		WaveAttackStruct[8].y = y;
		WaveAttackStruct[9].y = y-4;
	}
}


void WaveAttack(void){
	uint32_t i,j = 0;
	uint32_t speed=3;
	if(waveattackflag >= 1 && wave_wait == 1){ // use flag to time attacks
		if(WaveAttackStruct[wavei].moving == 0){ // if not moving already
				//WaveAttackStruct[wavei].x = sprite[1].x; // put attack where it starts
				//WaveAttackStruct[wavei].y = sprite[1].y; 
				ST7735_DrawBitmap(WaveAttackStruct[wavei].x, WaveAttackStruct[wavei].y, bossattack, WaveAttackStruct[wavei].width,WaveAttackStruct[wavei].height);
				WaveAttackStruct[wavei].moving = 1; // set the attack to moving
				wavei++;
				waveattackflag = 0;
		}
	}
	for(i=0;i<WaveAttackStructSize;i++){
		for(j=0;j<speed;j++){
			if(WaveAttackStruct[i].moving == 1){
				WaveAttackStruct[i].x = WaveAttackStruct[i].x + WaveAttackStruct[i].xvelocity; // move the attack
				WaveAttackStruct[i].y = WaveAttackStruct[i].y + WaveAttackStruct[i].yvelocity; 
				ST7735_DrawBitmap(WaveAttackStruct[i].x, WaveAttackStruct[i].y, bossattack, WaveAttackStruct[i].width,WaveAttackStruct[i].height);	
				
				if(CheckCollision(&sprite[0], &WaveAttackStruct[i]) == 1){
					WaveAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(WaveAttackStruct[i].x, WaveAttackStruct[i].y, blackbossattack, WaveAttackStruct[i].width,WaveAttackStruct[i].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				else if(((WaveAttackStruct[i].xvelocity > 0) && (WaveAttackStruct[i].x > 128)) || ((WaveAttackStruct[i].yvelocity > 0) && (WaveAttackStruct[i].y > 160))){
					WaveAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(WaveAttackStruct[i].x, WaveAttackStruct[i].y, blackbossattack, WaveAttackStruct[i].width,WaveAttackStruct[i].height);	// when collides, disappear		
				}
			}
		}
	}
	if((wavei >= WaveAttackStructSize) && (WaveAttackStruct[WaveAttackStructSize-1].moving == 0)){ // game can only hold a number of attacks equal to StraightAttackStructSize
		wavei = 0;
		wave_wait = 1;
		WaveAttackInit();
	}
	if(wavei >= WaveAttackStructSize){
		wave_wait = 0;
	}
}


uint32_t beami = 0;
void BeamAttack(void){
	int32_t i,j = 0;
	uint32_t speed=2;
	if(beam_wait == 1 && beamattackflag >= 5 && (chosen_attack == BEAM || chosen_attack == ALL)){ // use flag to time attacks
		if(BeamAttackStruct[beami].moving == 0){ // if not moving already
				BeamAttackStruct[beami].x = sprite[3].x + sprite[3].width - 16; // place where attack starts
				BeamAttackStruct[beami].y = sprite[3].y + 8; 
				ST7735_DrawBitmap(BeamAttackStruct[beami].x, BeamAttackStruct[beami].y, BeamAttackStruct[beami].image, BeamAttackStruct[beami].width,BeamAttackStruct[beami].height);
				BeamAttackStruct[beami].moving = 1; // set the attack to moving
				beami++;
		}
		if(BeamAttackStruct[beami].moving == 0){ // if not moving already
				BeamAttackStruct[beami].x = sprite[3].x + sprite[3].width - 8; // place where attack starts
				BeamAttackStruct[beami].y = sprite[3].y + 8; 
				ST7735_DrawBitmap(BeamAttackStruct[beami].x, BeamAttackStruct[beami].y, BeamAttackStruct[beami].image, BeamAttackStruct[beami].width,BeamAttackStruct[beami].height);
				BeamAttackStruct[beami].moving = 1; // set the attack to moving
				beami++;
		}
		if(BeamAttackStruct[beami].moving == 0){ // if not moving already
				BeamAttackStruct[beami].x = sprite[3].x + sprite[3].width; // place where attack starts
				BeamAttackStruct[beami].y = sprite[3].y + 8; 
				ST7735_DrawBitmap(BeamAttackStruct[beami].x, BeamAttackStruct[beami].y, BeamAttackStruct[beami].image, BeamAttackStruct[beami].width,BeamAttackStruct[beami].height);
				BeamAttackStruct[beami].moving = 1; // set the attack to moving
				beami++;
		}
		beamattackflag = 0;
	}
	for(i=0;i<BeamAttackStructSize;i++){
		for(j=0;j<speed;j++){
			if(BeamAttackStruct[i].moving == 1){
				ST7735_DrawBitmap(BeamAttackStruct[i].x, BeamAttackStruct[i].y, BeamAttackStruct[i].dead, BeamAttackStruct[i].width, BeamAttackStruct[i].height);	
				BeamAttackStruct[i].x = BeamAttackStruct[i].x + BeamAttackStruct[i].xvelocity; // move the attack
				BeamAttackStruct[i].y = BeamAttackStruct[i].y + BeamAttackStruct[i].yvelocity; 
				ST7735_DrawBitmap(BeamAttackStruct[i].x, BeamAttackStruct[i].y, BeamAttackStruct[i].image, BeamAttackStruct[i].width,BeamAttackStruct[i].height);	
				
				if(CheckCollisionNoOffset(&sprite[0], &BeamAttackStruct[i]) == 1){
					BeamAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(BeamAttackStruct[i].x, BeamAttackStruct[i].y, BeamAttackStruct[i].dead, BeamAttackStruct[i].width,BeamAttackStruct[i].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				else if((BeamAttackStruct[i].x < 0) || (BeamAttackStruct[i].x > 128) || (BeamAttackStruct[i].y < 0) || (BeamAttackStruct[i].y > 154) ){
					BeamAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(BeamAttackStruct[i].x, BeamAttackStruct[i].y, BeamAttackStruct[i].dead, BeamAttackStruct[i].width,BeamAttackStruct[i].height);	// when collides, disappear
				}
			}
		}
	}
	if((beami >= BeamAttackStructSize) && (BeamAttackStruct[BeamAttackStructSize-1].moving == 0)){ // game can only hold a number of attacks equal to StraightAttackStructSize
		beami = 0;
		beam_wait = 1;
	}
	else if(beami >= BeamAttackStructSize){
		beam_wait = 0;
	}
}

uint32_t sprayi = 0;
void SprayAttack(void){
	int32_t i,j = 0;
	uint32_t speed=1;
	if(spray_wait == 1 && sprayattackflag >= 3 && (chosen_attack == SPRAY || chosen_attack == ALL)){ // use flag to time attacks
		if(SprayAttackStruct[sprayi].moving == 0){ // if not moving already
				SprayAttackStruct[sprayi].x = sprite[3].x + 10; // place where attack starts
				SprayAttackStruct[sprayi].y = sprite[3].y + 6; 
				ST7735_DrawBitmap(SprayAttackStruct[sprayi].x, SprayAttackStruct[sprayi].y, SprayAttackStruct[sprayi].image, SprayAttackStruct[sprayi].width,SprayAttackStruct[sprayi].height);
				SprayAttackStruct[sprayi].moving = 1; // set the attack to moving
				sprayi++;
		}
		sprayattackflag = 0;
	}
	for(i=0;i<SprayAttackStructSize;i++){
		for(j=0;j<speed;j++){
			if(SprayAttackStruct[i].moving == 1){
				ST7735_DrawBitmap(SprayAttackStruct[i].x, SprayAttackStruct[i].y, SprayAttackStruct[i].dead, SprayAttackStruct[i].width, SprayAttackStruct[i].height);	
				SprayAttackStruct[i].x = SprayAttackStruct[i].x + SprayAttackStruct[i].xvelocity; // move the attack
				SprayAttackStruct[i].y = SprayAttackStruct[i].y + SprayAttackStruct[i].yvelocity; 
				ST7735_DrawBitmap(SprayAttackStruct[i].x, SprayAttackStruct[i].y, SprayAttackStruct[i].image, SprayAttackStruct[i].width,SprayAttackStruct[i].height);	
				
				if(CheckCollisionNoOffset(&sprite[0], &SprayAttackStruct[i]) == 1){
					SprayAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(SprayAttackStruct[i].x, SprayAttackStruct[i].y, SprayAttackStruct[i].dead, SprayAttackStruct[i].width,SprayAttackStruct[i].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				else if((SprayAttackStruct[i].x < 0) || (SprayAttackStruct[i].x > 128) || (SprayAttackStruct[i].y < 0) || (SprayAttackStruct[i].y > 154) ){
					SprayAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(SprayAttackStruct[i].x, SprayAttackStruct[i].y, SprayAttackStruct[i].dead, SprayAttackStruct[i].width,SprayAttackStruct[i].height);	// when collides, disappear
				}
			}
		}
	}
	if((sprayi >= SprayAttackStructSize) && (SprayAttackStruct[SprayAttackStructSize-1].moving == 0)){ // game can only hold a number of attacks equal to StraightAttackStructSize
		sprayi = 0;
		spray_wait = 1;
	}
	else if(sprayi >= SprayAttackStructSize){
		spray_wait = 0;
	}
}

uint32_t randsprayi = 0;
void RandSprayAttack(void){
	int32_t i,j = 0;
	uint32_t speed = 1;
	if(rand_spray_wait == 1 && randsprayattackflag >= 2 && (chosen_attack == RSPRAY || chosen_attack == ALL)){ // use flag to time attacks
		if(RandSprayAttackStruct[randsprayi].moving == 0){ // if not moving already
				RandSprayAttackStruct[randsprayi].x = sprite[3].x + 10 + Rand_2from0() + Rand_2from0(); // place where attack starts
				RandSprayAttackStruct[randsprayi].y = sprite[3].y + 6; 
				RandSprayAttackStruct[randsprayi].xvelocity = Rand_1from0();
				RandSprayAttackStruct[randsprayi].moving = 1; // set the attack to moving
				randsprayi++;
		}
		randsprayattackflag = 0;
	}
	for(i=0;i<RandSprayAttackStructSize;i++){
		for(j=0;j<speed;j++){
			if(RandSprayAttackStruct[i].moving == 1){
				ST7735_DrawBitmap(RandSprayAttackStruct[i].x, RandSprayAttackStruct[i].y, RandSprayAttackStruct[i].dead, RandSprayAttackStruct[i].width, RandSprayAttackStruct[i].height);	
				RandSprayAttackStruct[i].x = RandSprayAttackStruct[i].x + RandSprayAttackStruct[i].xvelocity; // move the attack
				RandSprayAttackStruct[i].y = RandSprayAttackStruct[i].y + RandSprayAttackStruct[i].yvelocity; 
				ST7735_DrawBitmap(RandSprayAttackStruct[i].x, RandSprayAttackStruct[i].y, RandSprayAttackStruct[i].image, RandSprayAttackStruct[i].width,RandSprayAttackStruct[i].height);	
				
				if(CheckCollisionNoOffset(&sprite[0], &RandSprayAttackStruct[i]) == 1){
					RandSprayAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
					ST7735_DrawBitmap(RandSprayAttackStruct[i].x, RandSprayAttackStruct[i].y, RandSprayAttackStruct[i].dead, RandSprayAttackStruct[i].width,RandSprayAttackStruct[i].height);	// when collides, disappear
					PlayerTakeDamage(); // player takes damage
				}
				else if((RandSprayAttackStruct[i].x < 0) || (RandSprayAttackStruct[i].x > 128) || (RandSprayAttackStruct[i].y < 0) || (RandSprayAttackStruct[i].y > 154) ){
					RandSprayAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
					ST7735_DrawBitmap(RandSprayAttackStruct[i].x, RandSprayAttackStruct[i].y, RandSprayAttackStruct[i].dead, RandSprayAttackStruct[i].width, RandSprayAttackStruct[i].height);	// when collides, disappear
				}
			}
		}
	}
	if((randsprayi >= RandSprayAttackStructSize) && (RandSprayAttackStruct[RandSprayAttackStructSize-1].moving == 0)){ // game can only hold a number of attacks equal to StraightAttackStructSize
		randsprayi = 0;
		rand_spray_wait = 1;
	}
	else if(randsprayi >= RandSprayAttackStructSize){
		rand_spray_wait = 0;
	}
}

//void TracerAttackInit(void){
//	uint32_t i,x,x2,y,y2,xstart;
//	x = xRandom();
//	x2 = xRandom();
//	y = yRandom();
//	y2 = yRandom();
//	TracerAttackStruct[0].x = 0;
//	TracerAttackStruct[0].y = y;
//	TracerAttackStruct[1].x = 0;
//	TracerAttackStruct[1].y = y2;
//	TracerAttackStruct[2].x = x;
//	TracerAttackStruct[2].y = 160;
//	TracerAttackStruct[3].x = x2;
//	TracerAttackStruct[3].y = 160;
//	
//	
//}


//void TracerAttack(void){
//	uint32_t i;
//	if(tracerattackflag >10){
//		for(i=0;i<TracerAttackStructSize;i++){
//			ST7735_DrawBitmap(TracerAttackStruct[i].x, TracerAttackStruct[i].y, TracerAttackStruct[i].image, TracerAttackStruct[i].width,TracerAttackStruct[i].height);	// when collides, disappear		
//		}
//	}
//	if(tracerattackflag >20){
//		for(i=0;i<TracerAttackStructSize;i++){
//			ST7735_DrawBitmap(TracerAttackStruct[i].x, TracerAttackStruct[i].y, TracerAttackStruct[i].image, TracerAttackStruct[i].width,TracerAttackStruct[i].height);	// when collides, disappear		
//			if(CheckCollision(&sprite[0], &TracerAttackStruct[i]) == 1){
//					TracerAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
//					ST7735_DrawBitmap(TracerAttackStruct[i].x, TracerAttackStruct[i].y, TracerAttackStruct[i].dead, TracerAttackStruct[i].width,TracerAttackStruct[i].height);	// when collides, disappear
//					sprite[0].health--; // player takes damage
//			}
//		}
//	}
//}

uint32_t calculateScore(void){
	// if player lost calculate score based on what bosses they killed
	if(game_status == 4){
		return 50*boss1_dead + 150*boss2_dead + 300*boss3_dead;
	}
	// if player won, add remaining hp and remaining power ups to score
	else{
		return 50*boss1_dead + 150*boss2_dead + 300*boss3_dead + 10*sprite[0].health + 100*piazza_counter;
	}
}

//clear attacks but dont change counter
void ResetAttacks(void){
	ClearAttacks();
	piazza_counter++;
}

void CheckEnd(void){
	if(check_end_flag == 0 || game_status == 8){
		check_end_flag = 1;
		return;
	}
		ST7735_SetCursor(0,0);
		ST7735_OutPhrase(GRADE);
		if(sprite[0].health < 100){
			LCD_OutDec(0);
		}
		if(sprite[0].health < 10){
			LCD_OutDec(0);
		}
		LCD_OutDec(sprite[0].health);
		ST7735_SetCursor(0,1);
		ST7735_OutPhrase(BOSS);
		if(game_status == 2){
		if(sprite[1].health < 10){
			LCD_OutDec(0);
		}
		LCD_OutDec(sprite[1].health);
	}
		else if(game_status == 1){
			if(sprite[2].health < 10){
				LCD_OutDec(0);
			}
			LCD_OutDec(sprite[2].health);
		}
		else if(game_status == 3){
			if(sprite[3].health < 10){
				LCD_OutDec(0);
			}
				LCD_OutDec(sprite[3].health);
		}
		
		if(sprite[0].health <= 0){ // if your health becomes 0
			ST7735_FillScreen(0x00);
			ST7735_SetCursor(2,5);
			ST7735_OutPhrase(LOSE);
			ST7735_SetCursor(2,7);
			ST7735_OutPhrase(SCORE);
			game_status = 4;
			LCD_OutDec(calculateScore());
			ST7735_SetCursor(0,9);
			ST7735_OutPhrase(RESET);
			ST7735_SetCursor(0,10);
			ST7735_OutPhrase(RESET2);
			while(1){}
		}
		if(game_status == 3 && sprite[3].health <= 0){ // if boss health becomes 0
			ST7735_FillScreen(0x00);
			ST7735_SetCursor(2,5);
			ST7735_OutPhrase(WIN);
			ST7735_SetCursor(2,7);
			ST7735_OutPhrase(SCORE);
			boss3_dead = 1;
			game_status = 5;
			LCD_OutDec(calculateScore());
			ST7735_SetCursor(0,9);
			ST7735_OutPhrase(RESET);
			ST7735_SetCursor(0,10);
			ST7735_OutPhrase(RESET2);
			while(1){}	
		}
		if(game_status == 2 && sprite[1].health <= 0){
			PiazzaStruct[1].x = sprite[1].x + sprite[1].width - 8; //spawn power up where boss died
			PiazzaStruct[1].y = sprite[1].y + 20;
			PiazzaStruct[1].moving = 1;
			ResetAttacks(); //clear attacks
			ST7735_DrawBitmap(sprite[1].x, sprite[1].y, sprite[1].dead, sprite[1].width, sprite[1].height); //clear boss
			boss2_dead = 1; //mark boss as defeated
			game_status = 7; //go to powerup round between boss 2 and 3
		}
		else if(game_status == 1 && sprite[2].health <= 0){
			PiazzaStruct[1].x = sprite[2].x + sprite[2].width - 8; //spawn power up where boss died
			PiazzaStruct[1].y = sprite[2].y + 4;
			ResetAttacks(); //clear attacks
			WaveAttackInit();
			ST7735_DrawBitmap(sprite[2].x, sprite[2].y, sprite[2].dead, sprite[2].width, sprite[2].height); //clear boss
			boss1_dead = 1; //mark boss as defeated
			game_status = 6; //go to powerup round between boss 1 and 2
		}
		check_end_flag = 0;
}

char p_counter;
void DrawPowerUps(void){
	if(check_end_flag == 0){
		return;
	}
	ST7735_DrawBitmap(PiazzaStruct[0].x, PiazzaStruct[0].y, PiazzaStruct[0].image, PiazzaStruct[0].width, PiazzaStruct[0].height);
	ST7735_SetCursor(18, 0);
	ST7735_OutString(":");
	p_counter = (char)piazza_counter + 0x30;
	ST7735_OutChar(p_counter);
}

void ClearAttacks(void){
	if(game_status==2){
	for(int i = 0; i<WaveAttackStructSize; i++){
		WaveAttackStruct[i].moving = 0; // the attack collides with the player sprite, make it not move anymore
		ST7735_DrawBitmap(WaveAttackStruct[i].x, WaveAttackStruct[i].y, WaveAttackStruct[i].dead, WaveAttackStruct[i].width,WaveAttackStruct[i].height);
	}
	WaveAttackInit();
	for(int i = 0; i<StraightAttackStructSize; i++){
		BossAttack[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(BossAttack[i].x, BossAttack[i].y, BossAttack[i].dead, BossAttack[i].width,BossAttack[i].height);
	}
}
	else if(game_status==1){
		for(int i = 0; i<ShineAttackStructSize; i++){
		ShineAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(ShineAttackStruct[i].x, ShineAttackStruct[i].y, ShineAttackStruct[i].dead, ShineAttackStruct[i].width,ShineAttackStruct[i].height);
	}
	}
	else if(game_status==3){
		for(int i = 0; i<SprayAttackStructSize; i++){
		SprayAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(SprayAttackStruct[i].x, SprayAttackStruct[i].y, SprayAttackStruct[i].dead, SprayAttackStruct[i].width,SprayAttackStruct[i].height);
	}
		for(int i = 0; i<BeamAttackStructSize; i++){
		BeamAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(BeamAttackStruct[i].x, BeamAttackStruct[i].y, BeamAttackStruct[i].dead, BeamAttackStruct[i].width,BeamAttackStruct[i].height);
	}
		for(int i = 0; i<RandSprayAttackStructSize; i++){
		RandSprayAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(RandSprayAttackStruct[i].x, RandSprayAttackStruct[i].y, RandSprayAttackStruct[i].dead, RandSprayAttackStruct[i].width,RandSprayAttackStruct[i].height);
	}
	for(int i = 0; i<CircleAttackStructSize; i++){
		CircleAttackStruct[i].moving = 0; // if the attack is offscreen, make it not move anymore
		ST7735_DrawBitmap(CircleAttackStruct[i].x, CircleAttackStruct[i].y, CircleAttackStruct[i].dead, CircleAttackStruct[i].width,CircleAttackStruct[i].height);
	}
		chosen_attack = NONE; //reset cooldown on valvano's attack
	}
	piazza_counter--;
}

void ST7735_OutPhrase(phrase_t message){
	ST7735_OutString((char *)Phrases[message][myLanguage]);
}

uint8_t dialogue_flag = 0;

int main(void){
  DisableInterrupts();
	PLL_Init(10);       // Bus clock is 9000000 HZ at 10
  Output_Init();
	PortE_Init();
	PortF_Init();
	ADC_Init();
 	DAC_Init();
	SysTick_Init();
	Timer0A_Init(0xFFFFFF);
	Timer1A_Init(3100); //11.025 kHz = 11025 = 9000000/x x=816
  EnableInterrupts();
	//TracerAttackInit();
	
	
	
	//ST7735_InitR(INITR_REDTAB); 
  ST7735_FillScreen(0x00);            // set screen to black
	while(game_status == 0 || game_status == -1 || game_status == 8){
	while(game_status == 0 && button() == 0 && button2() == 0){
			// TITLE SCREEN
		ST7735_SetCursor(5,2);
		ST7735_OutPhrase(WELCOME);
		ST7735_SetCursor(3,3);
		ST7735_OutPhrase(TITLE);
		ST7735_SetCursor(3,7);
		ST7735_OutPhrase(START);
		ST7735_SetCursor(3,9);
		ST7735_OutPhrase(LANGUAGES);
	  if(Convert(yADCMail) == 1){
			cursor = 0;
		}
		else if(Convert(yADCMail) == -1){
			cursor = 1;
		}
		if(cursor == 0){
			ST7735_SetCursor(2,7);
			ST7735_OutString(">");
			ST7735_SetCursor(2,9);
			ST7735_OutString(" ");
		}
		else{
			ST7735_SetCursor(2,9);
			ST7735_OutString(">");
			ST7735_SetCursor(2,7);
			ST7735_OutString(" ");
		}
	}
	while(game_status == -1 && button() == 0 && button2() == 0){
		ST7735_SetCursor(3,7);
		ST7735_OutString((char *)Phrases[LANGUAGE][English]);
		ST7735_SetCursor(3,9);
		ST7735_OutString((char *)Phrases[LANGUAGE][Spanish]);
	  if(Convert(yADCMail) == 1){
			cursor = 0;
		}
		else if(Convert(yADCMail) == -1){
			cursor = 1;
		}
		if(cursor == 0){
			ST7735_SetCursor(2,7);
			ST7735_OutString(">");
			ST7735_SetCursor(2,9);
			ST7735_OutString(" ");
		}
		else{
			ST7735_SetCursor(2,9);
			ST7735_OutString(">");
			ST7735_SetCursor(2,7);
			ST7735_OutString(" ");
		}
	}
		while(button()==1){
		}
		while(button2()==1){
		}
	if(game_status == 8){
		game_status = 1;
		click_sound = 1;
	}
	else if(game_status == 0 && cursor == 0){
		game_status = 1; //change this to skip tutorial
		click_sound = 1;
	}
	else if(game_status == 0 && cursor == 1){
		ST7735_FillScreen(0x00); 
		cursor = 0;
		game_status = -1;
		click_sound = 1;

	}
	else if(game_status == -1 && cursor == 0){
		myLanguage = English;
		cursor = 0;
		game_status = 0;
		click_sound = 1;

	}
	else if(game_status == -1 && cursor == 1){
		myLanguage = Spanish;
		cursor = 0;
		game_status = 0;
		click_sound = 1;
	}
	ST7735_FillScreen(0x00);  
}
	ST7735_FillScreen(0x00);   
	Random_Init(NVIC_ST_CURRENT_R);
  WaveAttackInit();
  BossInit();
		while(button()==1){
		}
		while(button2()==1){
		}
		
		// set it to fight first boss
		game_status = 1;
	
while(1){
		
	
		// ****SAMPLE ADC STARTS HERE
		//while(ADCStatus != 1){} // sample ADC
    xPosition = Convert(xADCMail); 
		ADCStatus = 0; // clear flag
	
		yPosition = Convert(yADCMail); 
		ADCStatus = 0; // clear flag
		// **** END OF SAMPLE ADC
	
		CheckEnd();
	  DrawPowerUps();
	 
		while(game_status == 8 && button() == 0 && button2() == 0 && dialogue_flag != 2){
			ST7735_DrawBitmap(sprite[4].x, sprite[4].y, sprite[4].image, sprite[4].width, sprite[4].height); //draw valvano
			ST7735_SetCursor(0,12);
			ST7735_OutPhrase(TAUNT1_1);
			ST7735_SetCursor(0,13);
			ST7735_OutPhrase(TAUNT1_2);
			if(button() == 1){
				dialogue_flag = 1;
				while(button()==1){
				}
				while(button2()==1){
				}
			}
			if(dialogue_flag == 1){
				ST7735_SetCursor(0,14);
				ST7735_OutPhrase(RESPONSE1);
				while(button() == 0 && button2() == 0){
				}
				dialogue_flag++;
			}
			
		}
		
	 if(game_status == 8){
		  ST7735_FillScreen(ST7735_BLACK);
			game_status = 3;
			click_sound = 1;
	 }
	
	  if(piazzaflag <= 10){
			piazzaflag++; //flag to act as cooldown between powerups
		}
	
		if(button2() == 1 && piazza_counter > 0 && piazzaflag > 10){
			ClearAttacks();
			piazza_sound = 1;
			piazzaflag = 0; //reset flag when using powerup
		}
		// ** DRAWING SPRITES STARTS HERE
		DrawPlayer();
		
		if(game_status == 1){
			DrawBoss1();
			ShineAttack();
		}
		
		//methods for second boss, if on that stage
		else if(game_status == 2){
			DrawBoss2();
			StraightAttack();
			WaveAttack();
		}
		//third boss
		else if(game_status == 3){
			beamattackflag++;
			sprayattackflag++;
			randsprayattackflag++;
			circleattackflag++;
			attack_counter++;
			if(attack_counter >= attack_cooldown){
				generateRandomAttack();
				attack_counter = 0;
				attack_cooldown = 60 + 10*Rand_2from0();
			}
			DrawBoss3();
			BeamAttack();
			SprayAttack();
			RandSprayAttack();
			CircleAttack();
		}
		else if((game_status == 6) || (game_status == 7)){
			DrawPowerUp();
		}
		DrawPlayerAttack();
		//TracerAttack();
		// ** DRAWING SPRITES ENDS HERE
	
}
	
	
	
	
	
		// ***** NEXT PART IS FOR TESTING
		//x
//		ST7735_SetCursor(0,4);
//		ST7735_OutString(" x ADC Value:");
//		
//		ST7735_SetCursor(0,6);
//    LCD_OutDec(xADCMail); // show adc value 
//		
//		//y
//		ST7735_SetCursor(0,8);
//		ST7735_OutString(" y ADC Value:");
//		
//		ST7735_SetCursor(0,10);
//		LCD_OutDec(yADCMail);
		//-************* END OF TESTING PART*********
	
		
	}


