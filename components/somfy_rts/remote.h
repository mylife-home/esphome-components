#pragma once

#include "command.h"
#include "esphome/core/hal.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace somfy_rts {

class SomfyRtsRemote : public Component {
public:
  SomfyRtsRemote(InternalGPIOPin *pin, uint32_t address);

  void setup() override;
  void dump_config() override;

private:
  void send_command(uint32_t address, Command command, uint16_t rolling_code, bool repeated);

  uint32_t address_;
  InternalGPIOPin *pin_;
  ESPPreferenceObject store_;
  uint16_t rolling_code_;
};

}  // namespace somfy_rts
}  // namespace esphome
