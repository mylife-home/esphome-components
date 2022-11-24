#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

namespace esphome {
namespace oem {

struct OemComputerData {
  float i_rms;
  float v_rms;
  float p_real;
};

class OemComputer : public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    // After the base sensor has been initialized
    return setup_priority::DATA - 1.0f;
  }

  void set_id(const std::string &id) { this->id_ = id; }
  const std::string &get_id() const { return this->id_; }

  void set_zero(float zero) { this->zero_ = zero; }
  void set_sample_duration(uint32_t sample_duration) { this->sample_duration_ = sample_duration; }
  void set_source(voltage_sampler::VoltageSampler *source) { this->source_ = source; }
  void set_burden_resistor_value(uint32_t burden_resistor_value) { this->burden_resistor_value_ = burden_resistor_value; }
  void set_ct_turns(uint32_t ct_turns) { this->ct_turns_ = ct_turns; }

  void add_on_update_callback(std::function<void(const OemComputerData&)> &&callback) { this->update_callback_.add(std::move(callback)); }

  // TODO: Link OemComputer with voltage sensor (to have V inst)

 protected:
  std::string id_;

  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;
  /// The sampling source to read values from.
  voltage_sampler::VoltageSampler *source_;
  /// Burden resistor value
  uint32_t burden_resistor_value_ = 1;
  /// CT turns
  uint32_t ct_turns_ = 1;
  /// Voltage measured when there is no current throught CT. Use VRef/2 (divisor bridge)
  float zero_ = 0;

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

  bool is_sampling_ = false;
  CallbackManager<void(const OemComputerData&)> update_callback_{};
};

}  // namespace oem
}  // namespace esphome