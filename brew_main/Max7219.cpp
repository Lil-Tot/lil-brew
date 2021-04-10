#include "Max7219.h"

Max7219::Max7219(int chipSelect, uint8_t numberOfDisplays) {
  this->chipSelect = chipSelect;
  this->numberOfDisplays = numberOfDisplays;
  
  SPI.begin();
  pinMode(this->chipSelect,OUTPUT);
  digitalWrite(this->chipSelect,HIGH);

  for(uint8_t i = 0; i < this->numberOfDisplays; i++) {
    this->SendSPI(OP_DISPLAYTEST, 0, i);
    this->SetScanLimit(0x07, i);
    this->SetIntensity(0x09, i);
    this->SetAllClear(i);
    this->SetShutDown(0x01, i);    
  }
}

void Max7219::SendSPI(uint8_t op_code, uint8_t data, uint8_t displayNumber){
  unsigned int spi_data;
  spi_data = op_code << 8;
  spi_data = spi_data | data;


  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));
  digitalWrite(this->chipSelect,LOW);

  for(int i = this->numberOfDisplays; i >= 0; i--){
    if(displayNumber == i) {
      SPI.transfer16(spi_data);
    }
    else{
      SPI.transfer16(OP_NOOP << 8);
    }
  }

  digitalWrite(this->chipSelect,HIGH);
  SPI.endTransaction();
}

void Max7219::SetDecodeMode(uint8_t decodeMode, uint8_t displayNumber){
  this->SendSPI(OP_DECODE_MODE, decodeMode, displayNumber);
}

void Max7219::SetIntensity(uint8_t intensity, uint8_t displayNumber){
  this->SendSPI(OP_INTENSITY, intensity, displayNumber);
}

void Max7219::SetScanLimit(uint8_t limit, uint8_t displayNumber){
  this->SendSPI(OP_SCAN_LIMIT, limit, displayNumber);
}

void Max7219::SetShutDown(uint8_t mode, uint8_t displayNumber){
  this->SendSPI(OP_SHUTDOWN, mode, displayNumber);
}

void Max7219::SetHex(uint8_t digitPlace, uint8_t hex, bool dot, uint8_t displayNumber){
  if(dot)
    this->SendSPI(8 - digitPlace, hexMap[hex] | 0b10000000, displayNumber);
  else
    this->SendSPI(8 - digitPlace, hexMap[hex], displayNumber);
}

void Max7219::SetUnique(uint8_t digitPlace, uint8_t unique, bool dot, uint8_t displayNumber){
  if(dot)
    unique = unique | 0b10000000;
  this->SendSPI(8 - digitPlace, unique, displayNumber);
}

void Max7219::SetAllClear(uint8_t displayNumber){
  for(int i = 1; i<=8; i++) {
    this->SendSPI(i, 0x00, displayNumber);  
  }
}
