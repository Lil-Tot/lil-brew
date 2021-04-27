#include <SPI.h>
#include "Max7219.h"
#include "Max31855.h"
#include "BrewWatchController.h"
#include "TemperatureController.h"

#define CHIP_SELECT 15

#define TEMP_CLICK 1
#define TEMP_ENCODE_0 2
#define TEMP_ENCODE_1 3

#define TIMER_CLICK  10
#define TIMER_ENCODE_0 0
#define TIMER_ENCODE_1 4
#define HEATER 5

unsigned long tickTime;

Max31855 max31855(CHIP_SELECT);
Max7219 max7219(CHIP_SELECT, 2);

// BrewWatchController brewWatchController = BrewWatchController(TIMER_CLICK,
//                                                               TIMER_ENCODE_0,
//                                                               TIMER_ENCODE_1,
//                                                               &max7219,
//                                                               0
//                                                               );
TemperatureController temperatureController = TemperatureController(TIMER_CLICK,
                                                                TIMER_ENCODE_0,
                                                                TIMER_ENCODE_1,
                                                                &max7219,
                                                                &max31855,
                                                                1,
                                                                HEATER
                                                                );
void setup() {
  pinMode(CHIP_SELECT, OUTPUT);
  pinMode(HEATER, OUTPUT);
  digitalWrite(CHIP_SELECT, HIGH);

  SPI.begin();
  max7219.Begin();
  // brewWatchController.Begin();
  temperatureController.Begin();
}

void loop() {
  tickTime = millis();
  // brewWatchController.Tick(tickTime);
  temperatureController.Tick(tickTime);
}