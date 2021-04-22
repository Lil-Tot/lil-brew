#include "BrewWatchController.h"


BrewWatchController* BrewWatchController::brewWatchControllerList = NULL;

BrewWatchController::BrewWatchController(uint8_t encodeClick,
                                         uint8_t encodePin0,
                                         uint8_t encodePin1,
                                         Max7219 *max7219, 
                                         int displayNumber
                                         ){
    brewWatchControllerList = this;

    this->encodeClick = encodeClick;
    this->encodePin0 = encodePin0;
    this->encodePin1 = encodePin1;
    this->display = max7219;
    this->displayNumber = displayNumber;

    this->time = 0;
    this->stopTime = 0;
}

void BrewWatchController::Begin() {
    this->state = START;

    pinMode(this->encodeClick, INPUT_PULLUP);
    pinMode(this->encodePin0, INPUT_PULLUP);
    pinMode(this->encodePin1, INPUT_PULLUP);
    this->encoder = digitalRead(this->encodePin0) | digitalRead(this->encodePin1) << 1;

    attachInterrupt(digitalPinToInterrupt(this->encodeClick), this->callback_UpdateState, RISING);
}

void BrewWatchController::Tick(unsigned long tickTime){
    if(this->state == START || this->state == COUNT) {
        if(tickTime - this->secondsHand > 1000) {
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
        this->display->SetAllClear(this->displayNumber);
    }
    else{
        this->display->SetUnique(0,P, this->displayNumber);
        this->display->SetUnique(1,U, this->displayNumber);
        this->display->SetHex(2,0x5, this->displayNumber);
        this->display->SetUnique(3,H, this->displayNumber);

        this->display->SetHex(6,0xE, this->displayNumber);
        this->display->SetHex(7,0x1, this->displayNumber);
  }
  this->flash = ! this->flash;
}

void BrewWatchController::UpdateTime(){
    this->time += 1;

    int minutes = this->time/60;
    display->SetHex(0, minutes/10, this->displayNumber);
    display->SetHex(1, minutes%10, this->displayNumber, true);    
    
    int seconds = this->time%60;
    display->SetHex(2, seconds/10, this->displayNumber);
    display->SetHex(3, seconds%10, this->displayNumber);      
}

void BrewWatchController::UpdateStopTime(){
    this->encoder = (this->encoder << 2) | (digitalRead(this->encodePin0) | (digitalRead(this->encodePin1) << 1));
    this->encoder = this->encoder & 0b1111;

    if(this->encoder == 0b1101 || this->encoder == 0b0100 || this->encoder == 0b0010 || this->encoder == 0b1011)
        this->stopTime += .25; //clockwise movement
    if(this->encoder == 0b1110 || this->encoder == 0b0111 || this->encoder == 0b0001 || this->encoder == 0b1000) 
        this->stopTime -= .25; //counter-clockwise movement    
    
    if(this->stopTime >= 100)
        this->stopTime = 0;
    if(this->stopTime < 0)
        this->stopTime = 99;   
    
    this->display->SetHex(4, ((int)(this->stopTime/10)%10), this->displayNumber);
    this->display->SetHex(5, (int)this->stopTime%10, this->displayNumber, true);
    this->display->SetHex(6, 0, this->displayNumber);
    this->display->SetHex(7, 0, this->displayNumber);
}

ICACHE_RAM_ATTR void BrewWatchController::callback_UpdateStopTime(){
    brewWatchControllerList->UpdateStopTime();
}

void BrewWatchController::UpdateState(){
    if(this->state == START) {
        this->display->SetAllClear();

        this->display->SetUnique(0,t, this->displayNumber);
        this->display->SetUnique(1,u, this->displayNumber);
        this->display->SetUnique(2,r, this->displayNumber);
        this->display->SetUnique(3,n, this->displayNumber);

        attachInterrupt(digitalPinToInterrupt(this->encodePin0),
                        this->callback_UpdateStopTime,
                        CHANGE
                        );
        attachInterrupt(digitalPinToInterrupt(this->encodePin1),
                        this->callback_UpdateStopTime,
                        CHANGE
                        );

        this->state = CONFIGURE;
        return;
    }

    if(this->state == CONFIGURE) {
        detachInterrupt(digitalPinToInterrupt(this->encodePin0));
        detachInterrupt(digitalPinToInterrupt(this->encodePin1));

        
        this->state = COUNT;
        return;
    }

    if(this->state == COUNT) {
        this->stopTime = 0;
        this->time = 0;

        this->state = START;
        return;
    }
}

ICACHE_RAM_ATTR void BrewWatchController::callback_UpdateState(){
    static unsigned long last_interrupt_time = 0;

    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) 
    {
        brewWatchControllerList->UpdateState();
    }
    last_interrupt_time = interrupt_time;
}