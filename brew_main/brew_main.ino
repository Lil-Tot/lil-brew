#include "Max7219.h"
#include "BrewWatchController.h"

#define SLAVE_MAX7219 15

#define TIMER_CLICK  5
#define TIMER_ENCODE_0 4
#define TIMER_ENCODE_1 0

Max7219 max7219(SLAVE_MAX7219);
BrewWatchController brewWatchController = BrewWatchController(TIMER_CLICK,
                                                                TIMER_ENCODE_0,
                                                                TIMER_ENCODE_1,
                                                                &max7219,
                                                                0
                                                                );
void setup() {  
}

void loop() {
  brewWatchController.tick();
}