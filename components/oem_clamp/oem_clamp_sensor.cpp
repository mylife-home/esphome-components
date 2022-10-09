#include "oem_clamp_sensor.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem_clamp {

static const char *const TAG = "oem_clamp";

void OemClampSensor::dump_config() {
  LOG_SENSOR("", "CT Clamp Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sample Duration: %.2fs", this->sample_duration_ / 1e3f);
  LOG_UPDATE_INTERVAL(this);
}

void OemClampSensor::update() {
  // Update only starts the sampling phase, in loop() the actual sampling is happening.

  // Request a high loop() execution interval during sampling phase.
  this->high_freq_.start();

  // Set timeout for ending sampling phase
  this->set_timeout("read", this->sample_duration_, [this]() {
    this->is_sampling_ = false;
    this->high_freq_.stop();

    if (this->samples_count_ == 0) {
      // Shouldn't happen, but let's not crash if it does.
      this->publish_state(0);
      return;
    }

    const float average = this->samples_sum_ / this->samples_count_;

    ESP_LOGD(TAG, "'%s' - avg: %.3f, min: %.3f, max: %.3f, count: %d", this->name_.c_str(),
      average, this->samples_min_, this->samples_max_, this->samples_count_);
    this->publish_state(0); // for now we don't care
  });

  // Set sampling values
  this->samples_min_ = +INFINITY;
  this->samples_max_ = -INFINITY;
  this->samples_sum_ = 0;
  this->samples_count_ = 0;
  this->is_sampling_ = true;
}

void OemClampSensor::loop() {
  if (!this->is_sampling_)
    return;

  // Perform a single sample
  float value = this->source_->sample();
  if (std::isnan(value))
    return;

  this->samples_min_ = std::min(this->samples_min_, value);
  this->samples_max_ = std::max(this->samples_max_, value);
  this->samples_sum_ += value;
  ++this->samples_count_;
}

}  // namespace oem_clamp
}  // namespace esphome