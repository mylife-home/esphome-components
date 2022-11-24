#include "oem_computer.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem {

static const char *const TAG = "oem_computer";

void OemComputer::setup() {
  const float burden_resistor_value = static_cast<float>(this->ct_config_.burden_resistor_value);
  const float ct_turns = static_cast<float>(this->ct_config_.ct_turns);
  this->ct_config_.ratio = ct_turns / burden_resistor_value;

  const float transfo_sec = this->v_config_.transfo_sec;
  const float r1 = static_cast<float>(this->v_config_.r1);
  const float r2 = static_cast<float>(this->v_config_.r2);
  this->v_config_.ratio = (230.f / transfo_sec) * (r1 + r2) / r1;
}

void OemComputer::dump_config() {
  ESP_LOGCONFIG(TAG, "OEM Computer '%s':", this->id_.c_str());
  ESP_LOGCONFIG(TAG, "  Sample duration: %.2fs", this->sample_duration_ / 1e3f);
  ESP_LOGCONFIG(TAG, "  VREF zero point: %.2fV", this->zero_);
  ESP_LOGCONFIG(TAG, "  CT:");
  ESP_LOGCONFIG(TAG, "    Burden resistor value: %dΩ", this->ct_config_.burden_resistor_value);
  ESP_LOGCONFIG(TAG, "    CT turns: %d", this->ct_config_.ct_turns);
  ESP_LOGCONFIG(TAG, "    Computer ratio: %.3f", this->ct_config_.ratio);
  ESP_LOGCONFIG(TAG, "    Max RMS current: %.1fA", this->zero_ / this->ct_config_.burden_resistor_value * this->ct_config_.ct_turns * 0.707);
  ESP_LOGCONFIG(TAG, "  V:");
  ESP_LOGCONFIG(TAG, "    Transfo sec: %.2fV", this->v_config_.transfo_sec);
  ESP_LOGCONFIG(TAG, "    R1: %dΩ", this->v_config_.r1);
  ESP_LOGCONFIG(TAG, "    R2: %dΩ", this->v_config_.r2);
  ESP_LOGCONFIG(TAG, "    Computer ratio: %.3f", this->v_config_.ratio);
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
    const float v_rms = std::sqrt(this->sampling_data_.v_sum_square / this->sampling_data_.count);
    const float i_rms = std::sqrt(this->sampling_data_.i_sum_square / this->sampling_data_.count);
    const float p_real = this->sampling_data_.p_sum / this->sampling_data_.count;

    constexpr float period_duration = 20.f; // 20 ms
    const float samples_per_period = this->sampling_data_.count * period_duration / this->sample_duration_;

    ESP_LOGD(TAG, "'%s' - v_rms=%.1fV, i_rms=%.3fA, p_real=%.1fW (sample count: %d, sample per period: %.2f)",
      this->id_.c_str(), v_rms, i_rms, p_real, this->sampling_data_.count, samples_per_period);

    this->update_callback_.call(OemComputerData{ .i_rms = i_rms, .v_rms = v_rms, .p_real = p_real });
  });

  // Init sampling values
  this->sampling_data_.count = 0;
  this->sampling_data_.i_sum_square = 0;
  this->sampling_data_.v_sum_square = 0;
  this->sampling_data_.p_sum = 0;
  this->is_sampling_ = true;
}

void OemComputer::loop() {
  if (!this->is_sampling_)
    return;

  // Perform a single sample
  const float ct_raw = this->ct_config_.sensor->sample();
  const float v_raw = this->v_config_.sensor->sample();
  if (std::isnan(ct_raw) || std::isnan(v_raw))
    return;

  // Get value relative to estimated 0 (VREF/2) + with ratio
  const float ct_measure = (ct_raw - this->zero_) * this->ct_config_.ratio;
  const float v_measure = (v_raw - this->zero_) * this->v_config_.ratio;

  ++this->sampling_data_.count;
  this->sampling_data_.i_sum_square += ct_measure * ct_measure;
  this->sampling_data_.v_sum_square += v_measure * v_measure;
  this->sampling_data_.p_sum += v_measure * ct_measure;
}

}  // namespace oem
}  // namespace esphome