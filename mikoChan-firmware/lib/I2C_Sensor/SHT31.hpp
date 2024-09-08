#ifndef SHT31_CLASS_H
#define SHT31_CLASS_H

#include <utility/I2C_Class.hpp>

class SHT31_Class : public m5::I2C_Device {
  public:
    static constexpr std::uint8_t DEFAULT_ADDRESS = 0x45;

    SHT31_Class(std::uint8_t i2c_addr = DEFAULT_ADDRESS, std::uint32_t freq = 400000, m5::I2C_Class* i2c = &m5::In_I2C)
        : m5::I2C_Device(i2c_addr, freq, i2c) {}

    /* bool begin(void); */
    void BussReset(void);
    void SoftReset(void);
    void Heater(uint8_t onoff);
    uint16_t ReadStatus(void);
    void GetTempHum(void);
    float Temperature(void);
    float Humidity(void);
    float DI(void);

  private:
    void writeRegister16(std::uint16_t data);
    /* void readRegister16(uint8_t data[],uint8_t num); */
    float humidity, temperature;
};

#endif  // !SHT31_CLASS_H
