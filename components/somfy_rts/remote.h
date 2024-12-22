#pragma once

#include "command.h"
#include "frame.h"
#include "esphome/core/hal.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace somfy_rts {

class SomfyRtsRemote : public Component {
public:
  SomfyRtsRemote(GPIOPin *pin, uint32_t address, uint32_t repeat);

  void setup() override;
  void dump_config() override;

private:
  void send_command(Command command);
  void send_frame(const Frame &frame, bool repeated);
  void send_pulse(bool state, uint32_t micros);
  uint16_t next_rolling_code();

  uint32_t address_;
  GPIOPin *pin_;
  ESPPreferenceObject store_;
  uint16_t rolling_code_;
  uint32_t repeat_;
};

}  // namespace somfy_rts
}  // namespace esphome
