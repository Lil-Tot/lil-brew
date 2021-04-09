#include <Arduino.h>

#ifndef Max7219_h
#define Max7219_h

class Max7219
{
private:
    uint8_t chipSelect;
    uint8_t addr[8] = {0};
    uint8_t hexMap[16] = { 0b01111110, 0b00110000, 0b01101101, 0b01111001,
                        0b00110011, 0b01011011, 0b01011111, 0b01110000,
                        0b01111111, 0b01110011, 0b01110111, 0b00011111,
                        0b01001110, 0b01111110, 0b01001111, 0b01000111 };
public:
    /*
    * Instaniate new Max7219, sets digits 0, min brightness, no decode, display all digits
    * Params:
    * chipSelect  pin for MOSI
    */
    Max7219(int chipSelect);

    /*
    * Send 2 bytes via SPI to Max7219
    * Params:
    * opcode    operations code to select register
    * data      byte of data to send to register
    */
    void SendSPI(uint8_t op_code, uint8_t data);

    /*
    * Sets decode mode (none or Code B)
    * Param:
    * decodeMode    0x00(none), 0x01(Code B Digit 0), 0x0F (Code B Digit 3-0), 0xFF (Code B)
    */
    void SetDecodeMode(uint8_t decodeMode);

    /*
    * Sets the screen brightness
    * Params:
    * intensity     0xX0(low) - 0xXF(Max)
    */
    void SetIntensity(uint8_t intensity);

    /*
    * Sets the scan limit (Digits to display)
    * Params:
    * limit     0xX0(low) - 0xXF(Max)
    */
    void SetScanLimit(uint8_t limit);
    
    /*
    * Sets shutdown mode
    * Params:
    * mode      0xX0(shutdown mode) - 0xX1(normal operation)
    */
    void SetShutDown(uint8_t mode);

    /*
    * Sets digits value
    * Params:
    * digitPlace    0x# (0-7)
    * value         0x# (0-F)
    * bool          True/False
    */
    void SetHex(uint8_t digitPlace, uint8_t hex, bool dot=false);

    /*
    * Sets a unique 7 segment diplay from user
    * Params:
    * digitPlace    0xX# (0-7)
    * unique        0b76543210 (0/1)
    *      D6
    *   D1    D5
    *      D0
    *   D2    D4
    *      D3    D7
    *  bool          true/false
    */
    void SetUnique(uint8_t digitPlace, uint8_t unique, bool dot=false);

    /*
    * Clears display
    */
    void SetAllClear();
};


#endif
