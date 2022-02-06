#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace pico_epanel {

class PicoEpanelOutput : public output::FloatOutput, public Component {
public:
  PicoEpanelOutput() = default;

  void set_index(uint8_t index) { this->index_ = index; }
  void add_on_write_callback(std::function<void(uint8_t, float)> &&callback) { this->on_write_callback_.add(std::move(callback)); }

protected:
  void write_state(float value) override {
    this->on_write_callback_.call(this->index_, value);
  }

private:
  uint8_t index_;
  CallbackManager<void(uint8_t, float)> on_write_callback_{};
};

}  // namespace pico_epanel
}  // namespace esphome
