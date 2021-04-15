#include "TemperatureController.h"

TemperatureController* TemperatureController::temperatureControllerList = NULL;

TemperatureController::TemperatureController(uint8_t encodeClick,
                                             uint8_t encodePin0,
                                             uint8_t encodePin1,
                                             Max7219 *max7219, 
                                             Max31855 *max31855,
                                             int displayNumber
                                             ){
        temperatureControllerList = this;

        this->encodeClick = encodeClick;
        this->encodePin0 = encodePin0;
        this->encodePin1 = encodePin1;
        this->display = max7219;
        this->temperatureProbe = max31855;
        this->displayNumber = displayNumber;
    }

void TemperatureController::Begin(){
    this->state = TEMP_READ;
    this->secondsHand = millis();

    // pinMode(this->encodeClick, INPUT_PULLUP);
    // pinMode(this->encodePin0, INPUT_PULLUP);
    // pinMode(this->encodePin1, INPUT_PULLUP);
    // this->encoder = digitalRead(this->encodePin0) | digitalRead(this->encodePin1) << 1;

    attachInterrupt(digitalPinToInterrupt(this->encodeClick), this->callback_UpdateState, RISING);
}

void TemperatureController::Tick(unsigned long tickTime){
    if(this->state == TEMP_START || this->state == TEMP_READ) {
        if(tickTime - this->secondsHand > 1000) {
            if(this->state == TEMP_START) {
                this->FlashPushE2();
            }
            if(this->state == TEMP_READ) {
                this->UpdateTemperature();
            }
            this->secondsHand = millis();
        }
    }
}

void TemperatureController::FlashPushE2(){
    if(this->flash){
        this->display->SetAllClear(this->displayNumber);
    }
    else{

        this->display->SetUnique(0,P, this->displayNumber);
        this->display->SetUnique(1,U, this->displayNumber);
        this->display->SetHex(2,0x5, this->displayNumber);
        this->display->SetUnique(3,H, this->displayNumber);

        this->display->SetHex(6,0xE, this->displayNumber);
        this->display->SetHex(7,0x2, this->displayNumber);
  }
  this->flash = ! this->flash;
}

void TemperatureController::UpdateTemperature(){
    double temperature = temperatureProbe->ReadFarenheit();
    Serial.print("Temperature is ");
    Serial.print(temperature);
    Serial.print("\n");

    this->display->SetHex(0, (int)(temperature/100), this->displayNumber); // 100
    this->display->SetHex(1, ((int)(temperature/10)%10), this->displayNumber); // 10
    this->display->SetHex(2, (int)temperature%10, this->displayNumber, true); // 1
    this->display->SetHex(3, (int)(temperature*10)%10, this->displayNumber); // .1

    //if(temperature != temperatureProbe)
}

void TemperatureController::UpdateTargetTemperature(){
    
}

ICACHE_RAM_ATTR  void TemperatureController::callback_UpdateTargetTemperature(){
    
}

void TemperatureController::UpdateState(){
    if(this->state == TEMP_START){
        this->display->SetAllClear();

        this->display->SetUnique(0,t, this->displayNumber);
        this->display->SetUnique(1,u, this->displayNumber);
        this->display->SetUnique(2,r, this->displayNumber);
        this->display->SetUnique(3,n, this->displayNumber);

        attachInterrupt(digitalPinToInterrupt(this->encodePin0),
                    this->callback_UpdateTargetTemperature,
                    CHANGE
                    );
        attachInterrupt(digitalPinToInterrupt(this->encodePin1),
                    this->callback_UpdateTargetTemperature,
                    CHANGE
                    );

    this->state = TEMP_READ;
    return;
    }
}

ICACHE_RAM_ATTR  void TemperatureController::callback_UpdateState(){
    temperatureControllerList->UpdateState();
}