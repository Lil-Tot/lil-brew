#include <Arduino.h>
#include "Max7219.h"

#define H 0b00110111
#define n 0b00010101
#define P 0b01100111
#define r 0b00000101
#define t  0b00001111
#define U 0b00111110
#define u 0b00011100

enum watchStates {
  START,
  CONFIGURE,
  COUNT
};

class BrewWatchController
{
  private:
    bool flash;
    uint8_t encoder;
    uint8_t encodeClick;
    uint8_t encodePin0;
    uint8_t encodePin1;
    uint8_t displayNumber;
    uint time;
    float stopTime;
    uint secondsHand;
    Max7219 *display;
    watchStates state;

    void FlashPushE1();
    void UpdateTime();
    void UpdateStopTime();
    void UpdateState();
      
  public:
    BrewWatchController(uint8_t encodeClick,
                        uint8_t encodePin0,
                        uint8_t encodePin1,
                        Max7219 *max7219, 
                        int displayNumber
                        );
    void Begin();

    static BrewWatchController *brewWatchControllerList;
    ICACHE_RAM_ATTR static void callback_UpdateState();
    ICACHE_RAM_ATTR static void callback_UpdateStopTime();

    void Tick(unsigned long tickTime);
};
