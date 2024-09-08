#include "MCP3425.hpp"
#include <esp_log.h>

bool MCP3425_Class::begin(bool convertMode, int bitMode, int pgaMode) {
    setting = (convertMode << 4) + (bitMode << 2) + pgaMode + (0b100 << 5);
    /* m5::I2C_Device::writeRegister8(0, setting); */
    _i2c->start(_addr, 0, _freq);
    _i2c->write(setting);
    _i2c->stop();
    /* writeRegister8(0x00, setting); */

    // Wire.beginTransmission(deviceAddress);
    // Wire.write(setting);
    // Wire.endTransmission();
    return true;
}

double MCP3425_Class::analogRead(){
    std::uint8_t result[2];
    readRegister(setting, result, sizeof(result));
    double val = ((result[0] << 8) + result[1]);
    /* if (val >= 32768) val -= 65536; */
    return val;
}
