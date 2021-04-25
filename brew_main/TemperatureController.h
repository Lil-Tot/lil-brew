#include <PID_v2.h>
#include <Arduino.h>
#include "Max7219.h"
#include "Max31855.h"

#define H 0b00110111
#define n 0b00010101
#define P 0b01100111
#define r 0b00000101
#define t  0b00001111
#define U 0b00111110
#define u 0b00011100

enum tempStates {
  TEMP_START,
  TEMP_CONFIGURE,
  TEMP_READ,
  TEMP_HEATER,
  TEMP_SET_K
};

class TemperatureController
{
    private:
        bool flash;
        uint8_t encoder;
        uint8_t encodeClick;
        uint8_t encodePin0;
        uint8_t encodePin1;
        uint8_t displayNumber;
        uint8_t HEATER;
        double temperature;
        uint secondsHand;
        double targetTemperature;
        Max7219 *display;
        Max31855 *temperatureProbe; //  could make this into a function pointer, cleaner
        tempStates state;

        //PID Controls
        double kp = 0;
        double ki = 0;
        double kd = 0;
        PID_v2 pid{this->kp, this->ki, this->kd, PID::Direct};
  
        unsigned long windowStartTime;
        const int windowSize = 5000;

        void FlashPushE2();
        void UpdateTemperature();
        void UpdateTargetTemperature();
        void UpdateState();

        void SetK();

    public:
        TemperatureController(uint8_t encodeClick,
                              uint8_t encodePin0,
                              uint8_t encodePin1,
                              Max7219 *max7219, 
                              Max31855 *max31855,
                              int displayNumber,
                              uint8_t HEATER
                              );
        void Begin();

        static TemperatureController *temperatureControllerList;
        ICACHE_RAM_ATTR static void callback_UpdateTargetTemperature();
        ICACHE_RAM_ATTR static void callback_UpdateState();

        void Tick(unsigned long tickTime);
};
