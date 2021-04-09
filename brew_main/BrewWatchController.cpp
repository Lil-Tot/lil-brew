#include "BrewWatchController.h"


BrewWatchController* BrewWatchController::brewWatchControllerList = NULL;

BrewWatchController::BrewWatchController(uint8_t encodeClick,
                                         uint8_t encodePin0,
                                         uint8_t encodePin1,
                                         Max7219 *max7219, 
                                         int displayNumber
                                         )
{
    brewWatchControllerList = this;

    this->encodeClick = encodeClick;
    this->encodePin0 = encodePin0;
    this->encodePin1 = encodePin1;
    this->display = max7219;
    this->displayNumber = displayNumber;

    this->state = START;
    this->time = 0;
    this->stopTime = 0;
    this->secondsHand = millis();

    pinMode(this->encodeClick, INPUT_PULLUP);
    pinMode(this->encodePin0, INPUT_PULLUP);
    pinMode(this->encodePin1, INPUT_PULLUP);
    this->encoder = digitalRead(TIMER_ENCODE_0) | digitalRead(TIMER_ENCODE_1) << 1;

    attachInterrupt(digitalPinToInterrupt(TIMER_CLICK), this->callback_UpdateState, RISING);
}

BrewWatchController::BrewWatchController() {}

void BrewWatchController::tick(){
    if(this->state == START || this->state == COUNT) {
        this->secondsHand;
        if(millis() - this->secondsHand > 1000) {
            if(this->state == START) {
                this->FlashPushE1();
            }
            if(this->state == COUNT) {
                this->UpdateTime();
            }
            this->secondsHand = millis();
        }
    }
}

void BrewWatchController::FlashPushE1(){
    if(this->flash){
        this->display->SetAllClear();
    }
    else{
        this->display->SetHex(0,0x5);
        this->display->SetHex(1,0xE);
        this->display->SetUnique(2,T);

        this->display->SetHex(4,0x0);
        this->display->SetHex(5,0x0, true);
        this->display->SetHex(6,0x0);
        this->display->SetHex(7,0x0);
  }
  this->flash = ! this->flash;
}

void BrewWatchController::UpdateTime(){
    time += 1;

    int minutes = time/60;
    display->SetHex(0, minutes/10);
    display->SetHex(1, minutes%10, true);    
    
    int seconds = time%60;
    display->SetHex(2, seconds/10);
    display->SetHex(3, seconds%10);      
}

void BrewWatchController::UpdateStopTime(){
    this->encoder = (this->encoder << 2) | (digitalRead(TIMER_ENCODE_0) | (digitalRead(TIMER_ENCODE_1) << 1));
    this->encoder = this->encoder & 0b1111;

    if(this->encoder == 0b1101 || this->encoder == 0b0100 || this->encoder == 0b0010 || this->encoder == 0b1011)
        this->stopTime += .25; //clockwise movement
    if(this->encoder == 0b1110 || this->encoder == 0b0111 || this->encoder == 0b0001 || this->encoder == 0b1000) 
        this->stopTime -= .25; //counter-clockwise movement    
    
    if(this->stopTime >= 100)
        this->stopTime = 0;
    if(this->stopTime < 0)
        this->stopTime = 99;   
    
    this->display->SetHex(4, ((int)(this->stopTime/10)%10));
    this->display->SetHex(5, (int)this->stopTime%10, true);
    this->display->SetHex(6, 0);
    this->display->SetHex(7, 0);
}

ICACHE_RAM_ATTR void BrewWatchController::callback_UpdateStopTime(){
    brewWatchControllerList->UpdateStopTime();
}

void BrewWatchController::UpdateState(){
    if(this->state == START) {
    Serial.print("Configure");
    this->display->SetAllClear();

    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0),
                    this->callback_UpdateStopTime,
                    CHANGE
                    );
    attachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1),
                    this->callback_UpdateStopTime,
                    CHANGE
                    );

    this->state = CONFIGURE;
    return;
  }

  if(this->state == CONFIGURE) {
    Serial.print("Count");

    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_0));
    detachInterrupt(digitalPinToInterrupt(TIMER_ENCODE_1));

    this->secondsHand = millis();
    
    this->state = COUNT;
    return;
  }

  if(this->state == COUNT) {
    Serial.print("Start");

    this->stopTime = 0;
    this->time = 0;
    this->secondsHand = millis();

    this->state = START;
    return;
  }
}

ICACHE_RAM_ATTR void BrewWatchController::callback_UpdateState(){
    brewWatchControllerList->UpdateState();
}