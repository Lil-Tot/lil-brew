#define USING_AXTLS
#include <Arduino.h>
#include <PID_v2.h>
#include "Max7219.h"
#include "Max31855.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecureAxTLS.h>

using namespace axTLS;

#define H 0b00110111
#define N 0b00010101
#define P 0b01100111
#define r 0b00000101
#define t  0b00001111
#define U 0b00111110
#define lower_u 0b00011100

enum tempStates {
  TEMP_START,
  TEMP_CONFIGURE,
  TEMP_READ,
  TEMP_HEATER,
  TEMP_BOIL
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
      double currentTemperature;
      Max7219 *display;
      Max31855 *temperatureProbe; // could make this into a function pointer, cleaner
      tempStates state;

      const char* host = "script.google.com";
      const String logURL = "https://script.google.com/macros/s/AKfycbxpMLPsV_okiCYWhbYaVyrA4W7jJ6xOMT3lJ9i3tC4A6MSaMJnLV-C0lTISjhExIqte/exec";
      String URL;
      WiFiClientSecure client;
      const char* ssid = "brew_net";
      const char* password = "secret_malt34";

      double kp = 0;
      double ki = 1000;
      double kd = 0;
      PID_v2 pid{this->kp, this->ki, this->kd, PID::Direct};

      unsigned long windowStartTime;
      const int windowSize = 5000;

      double slowPWM = 0;

      void FlashPushE2();
      void UpdateTemperature();
      void UpdateTargetTemperature();
      void UpdateState();

      void SetSlowPWM();

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