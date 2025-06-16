#pragma once

#include "command.h"
#include "frame.h"
#include "esphome/core/hal.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace somfy_rts {

class SomfyRtsRemote : public Component, public remote_base::RemoteTransmittable {
public:
  SomfyRtsRemote(uint32_t address, uint32_t repeat);

  void setup() override;
  void dump_config() override;

  void send_command(Command command);
  
private:
  void send_frame(remote_base::RemoteTransmitData *tdata, const uint8_t *data, bool repeated);
  uint16_t next_rolling_code();

  uint32_t address_;
  ESPPreferenceObject store_;
  uint16_t rolling_code_;
  uint32_t repeat_;
};

}  // namespace somfy_rts
}  // namespace esphome
