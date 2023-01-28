// protocol
// 16 inputs/16 outputs
// registers
// REG_CHECK => read = response magic 0x4242
// REG_RESET => write = set all outputs to 0, consider master has all inputs to 0 (so need to update if this is not the case)
// REG_INPUTS (u16) => read = each bit represent an input value
// REG_OUTPUTS (u8 x2) => write = first u8 = output number (0-15), second u8 = dimmer value (0-255)
// Note: interrupt pin? cf. https://www.sparkfun.com/products/15099

#pragma once

#include <bitset>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace pico_epanel {

class PicoEpanelOutput;
class SharedInterruptPin;

class PicoEpanelController : public Component, public i2c::I2CDevice {
public:
  PicoEpanelController(std::string id);

  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *intr_pin) { intr_pin_ = intr_pin; }

  void set_input(binary_sensor::BinarySensor *sensor, uint8_t index) { inputs_[index] = sensor; }
  void set_output(PicoEpanelOutput *output, uint8_t index);
  
  float read_internal_temp();

private:
  bool read_u16(uint8_t reg, uint16_t *value);
  bool write_u16(uint8_t reg, uint16_t value);

  std::bitset<16> read_inputs();
  void write_output(uint8_t index, uint8_t value);

  void refresh_inputs();

  std::string id_;
  std::array<binary_sensor::BinarySensor *, 16> inputs_{{nullptr}};
  InternalGPIOPin *intr_pin_{nullptr};
  SharedInterruptPin *owned_intr_pin_{nullptr}; // only set if we own it
  
};

}  // namespace pico_epanel
}  // namespace esphome
