#include "oem_clamp_sensor.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem_clamp {

static const char *const TAG = "oem_clamp";

void OemClampSensor::dump_config() {
  LOG_SENSOR("", "CT Clamp Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sample duration: %.2fs", this->sample_duration_ / 1e3f);
  ESP_LOGCONFIG(TAG, "  VREf zero point: %.2fV", this->zero_);
  ESP_LOGCONFIG(TAG, "  Burden resistor value: %dΩ", this->burden_resistor_value_);
  ESP_LOGCONFIG(TAG, "  CT turns: %d", this->ct_turns_);
  // https://tyler.anairo.com/projects/open-energy-monitor-calculator
  ESP_LOGCONFIG(TAG, "  Max RMS current: %.1fA", this->zero_ / this->burden_resistor_value_ * this->ct_turns_ * 0.707);
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

    if (this->sampling_data_.count == 0) {
      // Shouldn't happen, but let's not crash if it does.
      this->publish_state(0);
      return;
    }

    // End of sampling, get values

    // compute rms
    const float v_rms = std::sqrt(this->sampling_data_.measure_sum_square / this->sampling_data_.count);
    // primary current
    const float primary_current = v_rms / this->burden_resistor_value_ * this->ct_turns_;

    this->publish_state(primary_current);

    ESP_LOGD(TAG, "'%s' - v_rms=%.3fV, primary_current=%.3fA (sample count: %d)",
      this->name_.c_str(), v_rms, primary_current, this->sampling_data_.count);
  });

  // Init sampling values
  this->sampling_data_.count = 0;
  this->sampling_data_.raw_sum = 0;
  this->sampling_data_.measure_sum_square = 0;
  this->is_sampling_ = true;
}

void OemClampSensor::loop() {
  if (!this->is_sampling_)
    return;

  // Perform a single sample
  float raw = this->source_->sample();
  if (std::isnan(raw))
    return;

  // Get value relative to estimated 0 (VREF/2)
  const float measure = std::abs(raw - this->zero_);

  ++this->sampling_data_.count;
  this->sampling_data_.raw_sum += raw;
  this->sampling_data_.measure_sum_square += measure * measure;
}

}  // namespace oem_clamp
}  // namespace esphome