// protocol
// 16 inputs/16 outputs
// registers
// REG_CHECK => read = response magic 0x4242
// REG_RESET => write = set all outputs to 0, consider master has all inputs to 0 (so need to update if this is not the case)
// REG_INPUTS (u16) => read = each bit represent an input value
// REG_OUTPUTS (u8 x2) => write = first u8 = output number (0-15), second u8 = dimmer value (0-255)
// Note: interrupt pin? cf. https://www.sparkfun.com/products/15099

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace pico_epanel {

class PicoEpanelController : public Component, public i2c::I2CDevice {
public:
  PicoEpanelController() = default;

  void setup() override;
  void loop() override;
  void dump_config() override;

private:
  bool read_u16(uint8_t reg, uint16_t *value);
  bool write_u16(uint8_t reg, uint16_t value);

  uint16_t read_inputs();
  void write_output(uint8_t index, uint8_t value);

  uint16_t inputs_{0};
  std::array<uint8_t, 16> outputs_{{0}};
};

}  // namespace pico_epanel
}  // namespace esphome
