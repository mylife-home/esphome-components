#include "oem_computer.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem {

static const char *const TAG = "oem_computer";

void OemComputer::dump_config() {
  ESP_LOGCONFIG(TAG, "OEM Computer '%s':", this->id_.c_str());
  ESP_LOGCONFIG(TAG, "  Sample duration: %.2fs", this->sample_duration_ / 1e3f);
  ESP_LOGCONFIG(TAG, "  VREF zero point: %.2fV", this->zero_);
  ESP_LOGCONFIG(TAG, "  Burden resistor value: %dΩ", this->burden_resistor_value_);
  ESP_LOGCONFIG(TAG, "  CT turns: %d", this->ct_turns_);
  // https://tyler.anairo.com/projects/open-energy-monitor-calculator
  ESP_LOGCONFIG(TAG, "  Max RMS current: %.1fA", this->zero_ / this->burden_resistor_value_ * this->ct_turns_ * 0.707);
  LOG_UPDATE_INTERVAL(this);
}

void OemComputer::update() {
  // Update only starts the sampling phase, in loop() the actual sampling is happening.

  // Request a high loop() execution interval during sampling phase.
  this->high_freq_.start();

  // Set timeout for ending sampling phase
  this->set_timeout("read", this->sample_duration_, [this]() {
    this->is_sampling_ = false;
    this->high_freq_.stop();

    if (this->sampling_data_.count == 0) {
      // Shouldn't happen, but let's not crash if it does.
      // this->publish_state(0);
      return;
    }

    // End of sampling, get values

    // compute rms
    const float sensor_v_rms = std::sqrt(this->sampling_data_.measure_sum_square / this->sampling_data_.count);
    // primary current
    const float i_rms = sensor_v_rms / this->burden_resistor_value_ * this->ct_turns_;

    ESP_LOGD(TAG, "'%s' - sensor_v_rms=%.3fV, i_rms=%.3fA (sample count: %d)",
      this->id_.c_str(), sensor_v_rms, i_rms, this->sampling_data_.count);

    this->update_callback_.call(OemComputerData{ .i_rms = i_rms, .v_rms = 0, .p_real = 0 });
  });

  // Init sampling values
  this->sampling_data_.count = 0;
  this->sampling_data_.raw_sum = 0;
  this->sampling_data_.measure_sum_square = 0;
  this->is_sampling_ = true;
}

void OemComputer::loop() {
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

}  // namespace oem
}  // namespace esphome