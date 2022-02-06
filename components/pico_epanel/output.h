#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace pico_epanel {

class PicoEpanelOutput : public output::FloatOutput, public Component {
public:
  PicoEpanelOutput() = default;

  void add_on_write_callback(std::function<void(float)> &&callback) { this->on_write_callback_.add(std::move(callback)); }

protected:
  void write_state(float value) override {
    this->on_write_callback_.call(value);
  }

private:
  CallbackManager<void(float)> on_write_callback_{};
};

}  // namespace pico_epanel
}  // namespace esphome
