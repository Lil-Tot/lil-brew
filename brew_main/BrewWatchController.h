#include <Arduino.h>
#include "Max7219.h"

#define TIMER_CLICK  5
#define TIMER_ENCODE_0 4
#define TIMER_ENCODE_1 0

#define T  0b00001111

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
    BrewWatchController();

    BrewWatchController(uint8_t encodeClick,
                        uint8_t encodePin0,
                        uint8_t encodePin1,
                        Max7219 *max7219, 
                        int displayNumber
                        );

    static BrewWatchController *brewWatchControllerList;
    ICACHE_RAM_ATTR static void callback_UpdateState();
    ICACHE_RAM_ATTR static void callback_UpdateStopTime();

    void tick();
};
