#include <PID_v2.h>

#include "TemperatureController.h"

TemperatureController* TemperatureController::temperatureControllerList = NULL;

TemperatureController::TemperatureController(uint8_t encodeClick,
                                             uint8_t encodePin0,
                                             uint8_t encodePin1,
                                             Max7219 *max7219, 
                                             Max31855 *max31855,
                                             int displayNumber,
                                             uint8_t HEATER
                                             ){
    temperatureControllerList = this;

    this->encodeClick = encodeClick;
    this->encodePin0 = encodePin0;
    this->encodePin1 = encodePin1;
    this->display = max7219;
    this->temperatureProbe = max31855;
    this->displayNumber = displayNumber;
    this->HEATER = HEATER;

    this->targetTemperature = 150;
}

void TemperatureController::Begin(){
    this->state = TEMP_START;
    this->secondsHand = millis();

    pinMode(this->encodeClick, INPUT_PULLUP);
    pinMode(this->encodePin0, INPUT_PULLUP);
    pinMode(this->encodePin1, INPUT_PULLUP);
    this->encoder = digitalRead(this->encodePin0) | digitalRead(this->encodePin1) << 1;

    pinMode(HEATER, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(this->encodeClick), this->callback_UpdateState, RISING);
    
    // tell the PID to range between 0 and the full window size
    this->pid.SetOutputLimits(0, this->windowSize);

    this->pid.SetSampleTime(1000);
}

//Figure out what is wrong with pid if even
void TemperatureController::Tick(unsigned long tickTime){
    if(this->state == TEMP_START || this->state == TEMP_HEATER) {
        if(tickTime - this->secondsHand >= 1000) {
            if(this->state == TEMP_START) {
                this->FlashPushE2();
            }
            if(this->state == TEMP_HEATER){
                this->UpdateTemperature();
                const double output = this->pid.Run(this->temperatureProbe->ReadFarenheit());
               
                Serial.print("*** Time: ");
                Serial.print(tickTime);
                Serial.print("***\n");

                Serial.print("PID Output: ");
                Serial.print(output);
                Serial.print("\n");

                Serial.print("Target Temperature: ");
                Serial.print(this->targetTemperature);
                Serial.print("\n");

                Serial.print("Window Start Time: ");
                Serial.print(this->windowStartTime);
                Serial.print("\n \n");

                if(tickTime - this->windowStartTime > this->windowSize) {
                    // time to shift the Relay Window
                    windowStartTime = tickTime;
                }
                if (output > tickTime - windowStartTime){
                    digitalWrite(this->HEATER, HIGH);
                }
                else{
                    digitalWrite(this->HEATER, LOW);   
                }
            }
            this->secondsHand = millis();
        }
    }
    if(this->state == TEMP_SET_K) {
        this->SetK();
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

void TemperatureController::SetK(){
    this->kp = analogRead(A0)*(70.0/1023.0);
    Serial.print(this->kp);

    this->display->SetHex(5,(int)(this->kp/10)%10, this->displayNumber);
    this->display->SetHex(6,(int)this->kp%10, this->displayNumber, true);
    this->display->SetHex(7,(int)(this->kp*10)%10, this->displayNumber);
}

void TemperatureController::UpdateTemperature(){
    double temperature = this->temperatureProbe->ReadFarenheit();

    this->display->SetHex(0, (int)(temperature/100), this->displayNumber); // 100
    this->display->SetHex(1, ((int)(temperature/10)%10), this->displayNumber); // 10
    this->display->SetHex(2, (int)temperature%10, this->displayNumber, true); // 1
    this->display->SetHex(3, (int)(temperature*10)%10, this->displayNumber); // .1
}

void TemperatureController::UpdateTargetTemperature(){
    this->encoder = (this->encoder << 2) | (digitalRead(this->encodePin0) | (digitalRead(this->encodePin1) << 1));
    this->encoder = this->encoder & 0b1111;

    if(this->encoder == 0b1101 || this->encoder == 0b0100 || this->encoder == 0b0010 || this->encoder == 0b1011)
        this->targetTemperature += .25; //clockwise movement
    if(this->encoder == 0b1110 || this->encoder == 0b0111 || this->encoder == 0b0001 || this->encoder == 0b1000) 
        this->targetTemperature -= .25; //counter-clockwise movement    
    
    if(this->targetTemperature > 212)
        this->targetTemperature = 0;
    if(this->targetTemperature < 0)
        this->targetTemperature = 212;   
    
    this->display->SetHex(4, ((int)(this->targetTemperature/100)), this->displayNumber);
    this->display->SetHex(5, ((int)(this->targetTemperature/10)%10), this->displayNumber);
    this->display->SetHex(6, (int)this->targetTemperature%10, this->displayNumber, true);
    this->display->SetHex(7, 0, this->displayNumber);
}

ICACHE_RAM_ATTR  void TemperatureController::callback_UpdateTargetTemperature(){
    temperatureControllerList->UpdateTargetTemperature();
}

void TemperatureController::UpdateState(){
    if(this->state == TEMP_START){
        this->display->SetAllClear(this->displayNumber);

        this->display->SetHex(0,5, this->displayNumber);
        this->display->SetHex(1,0xE, this->displayNumber);
        this->display->SetUnique(2,t, this->displayNumber);


        this->state = TEMP_SET_K;
        return;
    }

    if(this->state == TEMP_SET_K){
        this->display->SetAllClear(this->displayNumber);

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

        this->state = TEMP_CONFIGURE;
        return;
    }

    if(this->state == TEMP_CONFIGURE) {
        detachInterrupt(digitalPinToInterrupt(this->encodePin0));
        detachInterrupt(digitalPinToInterrupt(this->encodePin1));

        this->pid.SetTunings(this->kp, this->ki, this->kd);

        // turn the PID on
        this->pid.Start(temperatureProbe->ReadFarenheit(),  // input
                0,                      // current output
                this->targetTemperature);                   // setpoint
        this->windowStartTime = millis();
        
        this->state = TEMP_HEATER;
        return;
    }

    if(this->state == TEMP_HEATER) {
        this->display->SetAllClear(this->displayNumber);

        digitalWrite(this->HEATER, LOW);
        
        this->state = TEMP_START;
        return;
    }
}

ICACHE_RAM_ATTR  void TemperatureController::callback_UpdateState(){
    static unsigned long last_interrupt_time = 0;

    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) 
    {
        temperatureControllerList->UpdateState();
    }
    last_interrupt_time = interrupt_time;
}
