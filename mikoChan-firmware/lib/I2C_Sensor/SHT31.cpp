#include "SHT31.hpp"

#include <unistd.h>
#define _msec *1000

/* bool SHT31_Class::begin(void){} */


void SHT31_Class::writeRegister16(std::uint16_t data){
    std::uint8_t buf[2] = {(std::uint8_t)(data>>8), (std::uint8_t)(data&0xFF)};
    writeRegister8Array(buf, sizeof(buf));
}

void SHT31_Class::BussReset(void){
    std::uint8_t data[2] = {0x00, 0x06};
    writeRegister8Array(data, sizeof(data));
}
void SHT31_Class::SoftReset(void){
    writeRegister16(0x30A2);
    usleep(500 _msec);
    writeRegister16(0x3041);
    usleep(500 _msec);
}
void SHT31_Class::Heater(uint8_t onoff){
    if (onoff) writeRegister16(0x306D);
    else writeRegister16(0x3066);
    usleep(500 _msec);
}
uint16_t SHT31_Class::ReadStatus(void){
    std::uint8_t data[3];
    writeRegister16(0xF32D);
    readRegister(_addr, data, sizeof(data));
    return (data[0]<<8|data[1]);
}
void SHT31_Class::GetTempHum(void){
    std::uint8_t data[7];

    writeRegister16(0x2400);
    usleep(300 _msec);
    readRegister(_addr, data, 6*sizeof(std::uint8_t));
    temperature = -45.0 + (175.0 * ((data[0] * 256.0) + data[1]) / 65535.0);
    humidity = (100.0 * ((data[3] * 256.0) + data[4])) / 65535.0;

}
float SHT31_Class::Temperature(void){
    return temperature;
}
float SHT31_Class::Humidity(void){
    return humidity;
}
float SHT31_Class::DI(void){
    return 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;
}

