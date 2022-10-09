#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

namespace esphome {
namespace oem_clamp {

class OemClampSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    // After the base sensor has been initialized
    return setup_priority::DATA - 1.0f;
  }

  void set_sample_duration(uint32_t sample_duration) { this->sample_duration_ = sample_duration; }
  void set_source(voltage_sampler::VoltageSampler *source) { this->source_ = source; }
  void set_burden_resistor_value(uint32_t burden_resistor_value) { this->burden_resistor_value_ = burden_resistor_value; }
  void set_ct_turns(uint32_t ct_turns) { this->ct_turns_ = ct_turns; }

 protected:
  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;
  /// The sampling source to read values from.
  voltage_sampler::VoltageSampler *source_;
  /// Burden resistor value
  uint32_t burden_resistor_value_ = 1; // avoid problems
  /// CT turns
  uint32_t ct_turns_ = 1; // avoid problems

  /**
   * https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino
   * 
   * - compute average to have a base 0 (since we get sin wave) => consider it will be constant, so only refresh it for the next sample
   * - compute rms voltage
   * - compute primary current, using burden value and CT turns
   */

  struct {
    float raw_sum;
    float measure_sum_square;
    uint32_t count;
  } sampling_data_;

  float measure_zero_ = 1.65; // by default use VRef/2, will be updated at next sampling
  bool is_sampling_ = false;
};

}  // namespace oem_clamp
}  // namespace esphome