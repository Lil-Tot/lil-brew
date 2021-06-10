#include "Max31855.h"

Max31855::Max31855(uint8_t chipSelect){
    this->chipSelect = chipSelect;
}

int32_t Max31855::ReadSPI(){
    int32_t data;

    SPI.beginTransaction(SPISettings(CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(this->chipSelect,LOW);

    data = SPI.transfer16(0x00);
    data = data << 16;
    data = data | SPI.transfer16(0x00);

    digitalWrite(this->chipSelect,HIGH);
    SPI.endTransaction();

    return data;
}

double Max31855::ReadCelcius(){
    int32_t data = this->ReadSPI();

    data = data >> 18;
    if (data & 0x2000) data |= 0xFFFE000;
    double centigrade = ((double)data) * .250;


    return centigrade;
}

double Max31855::ReadFarenheit(){
    return ReadCelcius()*1.8 + 32 + 5;
}
