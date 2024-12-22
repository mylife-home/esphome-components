#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/core/log.h"
#include "command.h"
#include "remote.h"

namespace esphome {
namespace somfy_rts {

class SomfyRtsOutput : public output::BinaryOutput, public Component {
public:
  SomfyRtsOutput(Command command, SomfyRtsRemote *remote)
  : command_(command)
  , remote_(remote) {
  }

protected:
  virtual void write_state(bool state) override {
    if (state) {
      this->remote_->send_command(this->command_);
    }
  }

private:
  Command command_;
  SomfyRtsRemote *remote_;
};

}  // namespace somfy_rts
}  // namespace esphome
