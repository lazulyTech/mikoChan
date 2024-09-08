#ifndef MCP3425_CLASS_H
#define MCP3425_CLASS_H

#include <utility/I2C_Class.hpp>

class MCP3425_Class : public m5::I2C_Device {
  public:
    static constexpr std::uint8_t DEFAULT_ADDRESS = 0x68;

    MCP3425_Class(std::uint8_t i2c_addr = DEFAULT_ADDRESS, std::uint32_t freq = 400000, m5::I2C_Class* i2c = &m5::In_I2C)
        : m5::I2C_Device(i2c_addr, freq, i2c) {}

    bool begin(bool convertMode=true, int bitMode=2, int pgaMode=0);
    /*--------------------------*/
    /*  convertMode             */
    /*      false: one shot     */
    /*    * true : consecutive  */
    /*  bitMode                 */
    /*         0 : 12 bit       */
    /*         1 : 14 bit       */
    /*    *    2 : 16 bit       */
    /*  pgaMode                 */
    /*    * 0 : x1              */
    /*      1 : x2              */
    /*      2 : x4              */
    /*      3 : x8              */
    /*--------------------------*/
    // default: 0b10011000

    double analogRead();

  private:
    int setting;
};

#endif
