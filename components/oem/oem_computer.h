#pragma once

// https://learn.openenergymonitor.org/electricity-monitoring/ac-power-theory/introduction

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
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;
  float get_setup_priority() const override {
    // After the base sensor has been initialized
    return setup_priority::DATA - 1.0f;
  }

  void set_id(const std::string &id) { this->id_ = id; }
  const std::string &get_id() const { return this->id_; }

  void set_sample_duration(uint32_t sample_duration) { this->sample_duration_ = sample_duration; }
  void set_zero(float zero) { this->zero_ = zero; }

  void set_ct_sensor(voltage_sampler::VoltageSampler *sensor) { this->ct_config_.sensor = sensor; }
  void set_ct_burden_resistor_value(uint32_t burden_resistor_value) { this->ct_config_.burden_resistor_value = burden_resistor_value; }
  void set_ct_turns(uint32_t ct_turns) { this->ct_config_.ct_turns = ct_turns; }

  void set_v_sensor(voltage_sampler::VoltageSampler *sensor) { this->v_config_.sensor = sensor; }
  void set_v_transfo_sec(float transfo_sec) { this->v_config_.transfo_sec = transfo_sec; }
  void set_v_r1(uint32_t r1) { this->v_config_.r1 = r1; }
  void set_v_r2(uint32_t r2) { this->v_config_.r2 = r2; }

  void add_on_update_callback(std::function<void(const OemComputerData&)> &&callback) { this->update_callback_.add(std::move(callback)); }

  // TODO: Link OemComputer with voltage sensor (to have V inst)

 protected:
  std::string id_;

  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;

  /// Voltage measured when there is no current throught CT. Use VRef/2 (divisor bridge), used for both current and voltage sensor
  float zero_ = 0;

  struct {
    /// The sampling source to read values from.
    voltage_sampler::VoltageSampler *sensor;

    /// Burden resistor value
    uint32_t burden_resistor_value;

    /// CT turns
    uint32_t ct_turns;

    /// Computed from burden_resistor_value and ct_turns
    float ratio;
  } ct_config_;

  struct {
    /// The sampling source to read values from.
    voltage_sampler::VoltageSampler *sensor;

    /// The secondary voltage of transformer (eg: 230V - 6V => 6V)
    float transfo_sec;

    /// R1 in the divider bridge
    uint32_t r1;

    /// R2 in the divider bridge
    uint32_t r2;

    /// Computed from v_transfo_sec, v_r1 and v_r2
    float ratio;
  } v_config_;

  struct {
    float i_sum_square;
    float v_sum_square;
    float p_sum;
    uint32_t count;
  } sampling_data_;

  bool is_sampling_ = false;
  CallbackManager<void(const OemComputerData&)> update_callback_{};
};

}  // namespace oem
}  // namespace esphome