#pragma once

#include <atomic>

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pico_epanel {

class PicoEpanelOutput : public output::FloatOutput, public Component {
public:
  PicoEpanelOutput() = default;

  void add_on_write_callback(std::function<void(float)> &&callback) { this->on_write_callback_.add(std::move(callback)); }

  void loop() override {
    bool has_pending_value = this->has_pending_value_.exchange(false);

    if (has_pending_value) {
      this->on_write_callback_.call(this->pending_value_);
    }
  }

protected:
  void write_state(float value) override {
    this->pending_value_ = value;
    this->has_pending_value_ = true;
  }

private:
  CallbackManager<void(float)> on_write_callback_{};
  float pending_value_;
  std::atomic<bool> has_pending_value_;
};

}  // namespace pico_epanel
}  // namespace esphome
