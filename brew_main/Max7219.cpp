#include <SPI.h>
#include "Max7219.h"

#define MAX7219_CLOCK 100000000

//Most signifigant byte does not matter for op-codes
#define OP_NOOP         0xF0
#define OP_DIGIT0       0x01
#define OP_DIGIT1       0x02
#define OP_DIGIT2       0x03
#define OP_DIGIT3       0x04
#define OP_DIGIT4       0x05
#define OP_DIGIT5       0x06
#define OP_DIGIT6       0x07
#define OP_DIGIT7       0x08
#define OP_DECODE_MODE  0x09 // Data 0=None/1=0/F=3-0/FF=7-0
#define OP_INTENSITY    0x0A // Data X0=Min/XF=Max
#define OP_SCAN_LIMIT   0x0B // Data X#=Digits
#define OP_SHUTDOWN     0x0C // Data X1=Display_On/X0=Display_Off
#define OP_DISPLAYTEST  0x0F // Data X0=off/X1=on


Max7219::Max7219(int chipSelect) {
  this->chipSelect = chipSelect;
  
  SPI.begin();
  pinMode(this->chipSelect,OUTPUT);
  digitalWrite(this->chipSelect,HIGH);

  this->SendSPI(OP_DISPLAYTEST, 0);
  this->SetScanLimit(0x07);
  this->SetIntensity(0x09);
  this->SetAllZeros();
  this->SetShutDown(0x01);
}

void Max7219::SendSPI(byte op_code, byte data) {
  unsigned int spi_data = 0;
  spi_data = op_code << 8;
  spi_data = spi_data | data;


  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));
  digitalWrite(this->chipSelect,LOW);
  SPI.transfer16(spi_data);
  digitalWrite(this->chipSelect,HIGH);
  SPI.endTransaction();
}

void Max7219::SetDecodeMode(byte decodeMode){
  this->SendSPI(OP_DECODE_MODE, decodeMode);
}

void Max7219::SetIntensity(byte intensity){
  this->SendSPI(OP_INTENSITY, intensity);
}

void Max7219::SetScanLimit(byte limit){
  this->SendSPI(OP_SCAN_LIMIT, limit);
}

void Max7219::SetShutDown(byte mode){
  this->SendSPI(OP_SHUTDOWN, mode);
}

void Max7219::SetHex(byte digitPlace, byte hex, bool dot){
  if(dot)
    this->SendSPI(8 - digitPlace, hexMap[hex] | 0b10000000);
  else
    this->SendSPI(8 - digitPlace, hexMap[hex]);
}

void Max7219::SetUnique(byte digitPlace, byte unique, bool dot){
  if(dot)
    unique = unique | 0b10000000;
  this->SendSPI(8 - digitPlace, unique);
}

void Max7219::SetAllZeros() {
  for(int i = 1; i<=8; i++) {
    this->SendSPI(i, 0x00);  
  }
}
