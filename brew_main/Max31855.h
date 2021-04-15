#include <Arduino.h>
#include <SPI.h>

#ifndef Max31855_h
#define Max31855_h

#define NEGATIVE 0x80000000

#define CLOCK 5000000

class Max31855{
    private:
        uint8_t chipSelect;

        int32_t ReadSPI(); 
    public:
        Max31855(uint8_t chipSelect);
        double ReadCelcius();
        double ReadFarenheit();
};

#endif