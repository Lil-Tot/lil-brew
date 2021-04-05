#include <arduino-timer.h>
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

enum timerStates timerController;
byte encoder;
float timerSetValue;
int timeBrewing = 0;
bool flash = false;

auto timers = timer_create_default();
Timer<>::Task flashTimeTask;
Timer<>::Task brewTimeTask; 

void setup() {
    Serial.begin(115200);
  /*
  pinMode(TEMP_CLICK, INPUT_PULLUP);
  pinMode(TEMP_ENCODE_0, INPUT_PULLUP);
  pinMode(TEMP_ENCODE_1, INPUT_PULLUP);
  */
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

  encoder = digitalRead(TIMER_ENCODE_0) | digitalRead(TIMER_ENCODE_1) << 1;

  // call the toggle_led function every 500 millis (half second)
  flashTimeTask = timers.every(1000, FlashSetTime, (void*) &flash);
}

void loop() {
  timers.tick();
}


// Maybe change to push E1
bool FlashSetTime(void *flash) {
  if(*(bool*)flash){
    max7219.SetAllClear();
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
  *(bool*)flash = ! *(bool*)flash;
  return true;
}

/*
void FlashSetTemperature(unsigned long timeNow){
  if(timeNow - flashTime > 1000){
    max7219.SetAllClear();
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
*/

bool BrewWatch(void* timeBrewing) {
  *(int*)timeBrewing += 1;

  int minutes = *(int*)timeBrewing/60;
  max7219.SetHex(0, minutes/10);
  max7219.SetHex(1, minutes%10, true);

  int seconds = *(int*)timeBrewing%60;
  max7219.SetHex(2, seconds/10);
  max7219.SetHex(3, seconds%10);

  return true;
}

ICACHE_RAM_ATTR void TimerEncodeUpdate(){
  encoder = (encoder << 2) | (digitalRead(TIMER_ENCODE_0) | (digitalRead(TIMER_ENCODE_1) << 1));
  encoder = encoder & 0b1111;

  if(encoder == 0b1101 || encoder == 0b0100 || encoder == 0b0010 || encoder == 0b1011)
    timerSetValue += .25; //clockwise movement
  if(encoder == 0b1110 || encoder == 0b0111 || encoder == 0b0001 || encoder == 0b1000) 
    timerSetValue -= .25; //counter-clockwise movement

  if(timerSetValue >= 100)
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
    max7219.SetAllClear();
    timers.cancel(flashTimeTask);
    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0), TimerEncodeUpdate, CHANGE);
    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1), TimerEncodeUpdate, CHANGE);

    timerController = SET_TIMER_SEC;
    return;
  }

  if(timerController == SET_TIMER_SEC) {
    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0));
    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1));

    brewTimeTask = timers.every(1000, BrewWatch, (void*) &timeBrewing);
    
    timerController = COUNTING;
    return;
  }

  if(timerController == COUNTING) {
    timerSetValue = 0;
    timers.cancel(brewTimeTask);
    flashTimeTask = timers.every(1000, FlashSetTime, (void*) &flash);
    timerController = START_TIMER;
    return;
  }
}


