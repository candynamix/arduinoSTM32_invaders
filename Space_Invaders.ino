/*
 *  Modified by JINsMac 
 *  Original game by 
 * 
 * 
 * digitalRead( USER_BTN ) 
 * 
 */






#include "Metro.h"  // <--  changed Metro.h
#include "Space_Invaders.h"
#include <Adafruit_GFX.h>
#include "BSP_f429i_discovery.h"
#include "TFT_F429_Discovery.h"

TFT_F429_Discovery display;

static unsigned char PROGMEM over[] =
{  
  B01100010,
  B10010100,
  B10011000,
  B10010000,
  B11111110,
  B00000000,
  B10000010,
  B10010010,
  B10010010,
  B10010010,
  B11111110,
  B00000000,
  B11111000,
  B00000100,
  B00000010,
  B00000100,
  B11111000,
  B00000000,
  B01111100,
  B10000010,
  B10000010,
  B10000010,
  B01111100 };

static unsigned char PROGMEM game[] =
{   
  B10000010,
  B10010010, 
  B10010010,
  B10010010,
  B11111110, 
  B00000000, 
  B11111110,
  B01000000,
  B00111000,
  B01000000, 
  B11111110,
  B00000000, 
  B00111110, 
  B01001000, 
  B10001000,
  B01001000,
  B00111110, 
  B00000000, 
  B01001110, 
  B10001010, 
  B10000010, 
  B10000010,
  B01111100 };

static const unsigned char PROGMEM pause[] = {
  B00000000,
  B00000000,
  B10000010,
  B10010010,
  B10010010,
  B10010010,
  B11111110,
  B00000000,
  B01001100,
  B10010010,
  B10010010,
  B10010010,
  B01100100,
  B00000000,
  B11111100,
  B00000010,
  B00000010,
  B00000010,
  B11111100,
  B00000000,
  B00111110,
  B01001000,
  B10001000,
  B01001000,
  B00111110,
  B00000000,
  B01100000,
  B10010000,
  B10010000,
  B10010000,
  B11111110,
  B00000000
};
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
///Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);  /// Original LCD --> blackwhite LCD panel 


///#define JPIN 8 //joystick button pin
///#define BPIN 2 //button pin


#define counter 1000 //debounce time

//Shift Register - aprinde cifra
///#define data 9
///#define stcp 10 
///#define shcp 11

//Shift Register - schimba ce cifra apare dintre cele 4
///#define dataB 12
///#define CP 13

#define nr_enemy 12
#define chance_shoot 4

unsigned int joyX; //joystick x value

char bounce; //the enemy bounces off the wall
char play = 0; // 0 - play, 1 - pause, 2 - defeat 

//byte chance_shoot = 4;
Enemy *enemy[nr_enemy];
Bullet *ShipBullet = new Bullet();
Bullet *EnemyBullet = new Bullet();
Ship *ship;

byte level = 1;
unsigned int score = 0;

Metro s = Metro(300), b = Metro(150), e = Metro(1000);
///Metro led = Metro(30, 1);

char oldB = 0;
char oldJB = 1;

void setup()   {
  Serial.begin(9600);
  //Buttons
//j  pinMode(BPIN, INPUT);
//j  pinMode(JPIN, INPUT);
  //Shift Register
//j  pinMode(data,OUTPUT);
//j  pinMode(stcp,OUTPUT);
//j  pinMode(shcp,OUTPUT);
//j  pinMode(dataB, OUTPUT);
//j  pinMode(CP, OUTPUT);
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  pinMode(USER_BTN,   INPUT); // mandatory for STM MCUs, on Arduino Uno working without

  display.begin();
  initGame(level);
}

void loop() {
  if (play == 0) { //play
    if(e.check() == 1) {  // moving Enemy location
      moveEnemy();
    }
    if(s.check() == 1) {
      buttonsPressed(); //check to see if any buttons have been pressed
      moveShip();
      if (ship->getLives() == 0) {
        play = 2;
      }
    }
    if(b.check()) {   // bullet location
      moveBullet();
      if (remaining_enemys() == 0) {
        level += 1;
        initGame(level);
      }
    }
  }
  else if (play == 1) { //pause
    buttonsPressed();
    showPause();
  }
  else { //play == 2 - defeat/dead
    showMenu();
///    displayScore(score);
    buttonsPressed();
  }
  
}

void addBullet(byte x, byte y, byte newSpeed, char side) {
  //side == 1 -> my ship, else enemy ship
  if (side == 1 && ShipBullet->getExist() == 0) {
    ShipBullet = new Bullet(x, y, newSpeed);
  }
  else if (side == 0 && EnemyBullet->getExist() == 0) {
    EnemyBullet = new Bullet(x, y, newSpeed);
  }
}

char getButton(char button, char oldB) {
  static short count[14] = {
    0,                                                                                                                                                         };
  static char prevB[14] = {
    0,                                                                                                                                                         };
  char b = digitalRead(button);
  if (b == prevB[button]) {
    count[button]++;
  }
  else {
    count[button] = 0;
  }
  prevB[button] = b;
  if (count[button] == counter) {
    count[button] = 0;
    return b;
  }
  else return prevB[button];
}

