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
    
    this->pid.SetOutputLimits(0, this->windowSize);

    this->pid.SetSampleTime(1000);

    WiFi.setPhyMode(WIFI_PHY_MODE_11B);
    WiFi.begin(this->ssid, this->password);
    delay(5000);
}

void TemperatureController::Tick(unsigned long tickTime){
    if(this->state == TEMP_START || this->state == TEMP_HEATER || this->state == TEMP_BOIL) {
        if(tickTime - this->secondsHand >= 1000) {
            if(this->state == TEMP_START) {
                this->FlashPushE2();
            }

            if(this->state == TEMP_HEATER || this->state == TEMP_BOIL){
                this->UpdateTemperature();
            }

            if(this->state == TEMP_HEATER){
                const double output = this->pid.Run(this->currentTemperature);
                if(tickTime - this->windowStartTime > this->windowSize) {
                    windowStartTime = tickTime;
                }
                if (output > tickTime - windowStartTime){
                    digitalWrite(this->HEATER, HIGH);
                }
                else{
                    digitalWrite(this->HEATER, LOW);   
                }

                this->URL = logURL;
                this->URL += "?Temp=";
                this->URL += currentTemperature;
                this->URL += "&Time=";
                this->URL += tickTime;
                this->URL += "&Kp=";
                this->URL += this->kp;
                this->URL += "&Output=";
                this->URL += output;
                
                client.connect(host, 443);
                client.print(String("GET ") + URL + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: BuildFailureDetectorESP8266\r\n" +
                "Connection: close\r\n\r\n"
                );
            }
            if(this->state == TEMP_BOIL){
                this->SetSlowPWM();  
                if(tickTime - this->windowStartTime > this->windowSize) {
                    windowStartTime = tickTime;
                }
                if (this->slowPWM > tickTime - windowStartTime){
                    digitalWrite(this->HEATER, HIGH);
                }
                else{
                    digitalWrite(this->HEATER, LOW);   
                }
            }
            this->secondsHand = tickTime;
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

void TemperatureController::SetSlowPWM(){
    this->slowPWM = analogRead(A0)*(this->windowSize/1023.0);

    double percentage = this->slowPWM/this->windowSize;

    this->display->SetHex(5,(int)(percentage), this->displayNumber);
    this->display->SetHex(6,(int)(percentage*10)%10, this->displayNumber);
    this->display->SetHex(7,(int)(percentage*100)%10, this->displayNumber);
}

void TemperatureController::UpdateTemperature(){
     this->currentTemperature = this->temperatureProbe->ReadFarenheit();

    this->display->SetHex(0, (int)(this->currentTemperature/100), this->displayNumber);
    this->display->SetHex(1, ((int)(this->currentTemperature/10)%10), this->displayNumber);
    this->display->SetHex(2, (int)this->currentTemperature%10, this->displayNumber, true);
    this->display->SetHex(3, (int)(this->currentTemperature*10)%10, this->displayNumber);
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

        this->display->SetUnique(0,t, this->displayNumber);
        this->display->SetUnique(1,lower_u, this->displayNumber);
        this->display->SetUnique(2,r, this->displayNumber);
        this->display->SetUnique(3,N, this->displayNumber);

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

        this->pid.Start(temperatureProbe->ReadFarenheit(),
                0,
                this->targetTemperature);
        this->windowStartTime = millis();
        
        this->state = TEMP_HEATER;
        return;
    }
    if(this->state == TEMP_HEATER) {
        this->display->SetAllClear(this->displayNumber);

        digitalWrite(this->HEATER, LOW);
        
        this->state = TEMP_BOIL;
        return;
    }

    if(this->state == TEMP_BOIL) {
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