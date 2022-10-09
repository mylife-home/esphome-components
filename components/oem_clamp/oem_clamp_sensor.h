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
  void set_burden_resistor_value(float burden_resistor_value) { this->burden_resistor_value_ = burden_resistor_value; }
  void set_ct_turns(float ct_turns) { this->ct_turns_ = ct_turns; }

 protected:
  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;
  /// The sampling source to read values from.
  voltage_sampler::VoltageSampler *source_;
  /// Burden resistor value
  float burden_resistor_value_;
  /// CT turns
  float ct_turns_;

  /**
   * https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino
   * 
   * 
   * 
   * 
   */

  float samples_min_ = +INFINITY;
  float samples_max_ = -INFINITY;
  float samples_sum_ = 0;
  uint32_t samples_count_ = 0;

  bool is_sampling_ = false;
};

}  // namespace oem_clamp
}  // namespace esphome