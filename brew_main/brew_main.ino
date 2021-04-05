#include "Max7219.h"

#define T  0b00001111
#define DEGREE  0b01100011

#define SLAVE_MAX7219 D8  // GPIO15
/*
#define TEMP_CLICK 1
#define TEMP_ENCODE_0 2
#define TEMP_ENCODE_1 3
*/
#define TIMER_CLICK  5
#define TIMER_ENCODE_0 4
#define TIMER_ENCODE_1 0


#define HEATER 9         // GPIO9

Max7219 max7219(SLAVE_MAX7219);

enum tempStates {
  START_TEMP,
  SET_TEMP,
  HEATER_ON,
  HEATER_OFF
};
enum timerStates {
  START_TIMER,
  SET_TIMER_SEC,
  COUNTING,
};

enum tempStates tempController;
byte tempEncoder;

unsigned long brewTime;
unsigned long timer;
unsigned long flashTime = 0;
enum timerStates timerController;
byte timerEncoder;
byte newTimerEncoder;
float timerSetValue = 0;
int timeBrewing = 0;

ICACHE_RAM_ATTR void TimerEncodeUpdate(){
  newTimerEncoder = digitalRead(TIMER_ENCODE_0) | (digitalRead(TIMER_ENCODE_1) << 1);

  byte sumEncoder = (timerEncoder << 2) | newTimerEncoder;

  if(sumEncoder == 0b1101 || sumEncoder == 0b0100 || sumEncoder == 0b0010 || sumEncoder == 0b1011)
    timerSetValue += .25; //clockwise movement
  if(sumEncoder == 0b1110 || sumEncoder == 0b0111 || sumEncoder == 0b0001 || sumEncoder == 0b1000) 
    timerSetValue -= .25; //counter-clockwise movement
  timerEncoder = newTimerEncoder;
  if(timerSetValue > 99)
    timerSetValue = 0;
  if(timerSetValue < 0)
    timerSetValue = 99;

  max7219.SetHex(4, ((int)(timerSetValue/10)%10));
  max7219.SetHex(5, (int)timerSetValue%10, true);
  max7219.SetHex(6, 0);
  max7219.SetHex(7, 0);
}


/*
void TempStateChange() {
  if(tempController == SET_TEMP) {
    tempController == HEATER_ON;
  }
  if(tempController == HEATER_ON || tempController == HEATER_OFF) {
    tempController = SET_TEMP;
  }
}
*/

ICACHE_RAM_ATTR void TimerStateChange() {
  if(timerController == START_TIMER) {
    max7219.SetAllZeros();
    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0), TimerEncodeUpdate, CHANGE);
    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1), TimerEncodeUpdate, CHANGE);

    timerController = SET_TIMER_SEC;

    return;
  }
  if(timerController == SET_TIMER_SEC) {
    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0));
    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1));
    
    timer = brewTime;

    //convert minutes to seconds
    timerSetValue = (int)timerSetValue*60;

    timerController = COUNTING;
    return;
  }
  if(timerController == COUNTING) {
    timerController = START_TIMER;
    return;
  }
}

void FlashSetTemperature(unsigned long timeNow){
  if(timeNow - flashTime > 1000){
    max7219.SetAllZeros();
    if(timeNow - flashTime > 2000){
      flashTime = brewTime;
    }
  }
  else{
  max7219.SetHex(0,0x5);
  max7219.SetHex(1,0xE);
  max7219.SetUnique(2,T);

  max7219.SetUnique(4,DEGREE);
  max7219.SetHex(5,0xF);
  }
}

void FlashSetTime(unsigned long timeNow){
  if(timeNow - flashTime > 1000){
    max7219.SetAllZeros();
    if(timeNow - flashTime > 2000){
      flashTime = brewTime;
    }
  }
  else{
    max7219.SetHex(0,0x5);
    max7219.SetHex(1,0xE);
    max7219.SetUnique(2,T);

    max7219.SetHex(4,0x0);
    max7219.SetHex(5,0x0, true);
    max7219.SetHex(6,0x0);
    max7219.SetHex(7,0x0);
  }
}

void setup() {
  /*
  pinMode(TEMP_CLICK, INPUT_PULLUP);
  pinMode(TEMP_ENCODE_0, INPUT_PULLUP);
  pinMode(TEMP_ENCODE_1, INPUT_PULLUP);
  */
  Serial.begin(115200);

  pinMode(TIMER_CLICK, INPUT_PULLUP);
  pinMode(TIMER_ENCODE_0, INPUT_PULLUP);
  pinMode(TIMER_ENCODE_1, INPUT_PULLUP);

  /*
  attachInterrupt(digitalPinToInterrupt(TEMP_CLICK), TimerStateChange, RISING);
  attachInterrupt(digitalPinToInterrupt(TEMP_CLICK), TimerStateChange, RISING);
  attachInterrupt(digitalPinToInterrupt(TEMP_CLICK), TempStateChange, CHANGE);
  */
  attachInterrupt(digitalPinToInterrupt(TIMER_CLICK), TimerStateChange, RISING);

  tempController = START_TEMP;
  timerController = START_TIMER;

  timerEncoder =  digitalRead(TIMER_ENCODE_1) << 1 | digitalRead(TIMER_ENCODE_0);
}

void loop() {
  brewTime = millis();

  if(timerController == START_TIMER) {
    FlashSetTime(brewTime);
  } 

  if(timerController == COUNTING) {
      if(brewTime - timer > 1000){
        timeBrewing += 1;

        int minutes = timeBrewing/60;
        max7219.SetHex(0, minutes/10);
        max7219.SetHex(1, minutes%10, true);

        int seconds = timeBrewing%60;
        max7219.SetHex(2, seconds/10);
        max7219.SetHex(3, seconds%10);
      
        timer = brewTime;
      }
    }
}
