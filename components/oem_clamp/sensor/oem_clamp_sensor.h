#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"
#include "esphome/components/oem_clamp/oem_clamp.h"

namespace esphome {
namespace oem_clamp {

enum SensorType { CURRENT = 0, VOLTAGE, APPARENT_POWER, REAL_POWER };

class OemClampSensor : public sensor::Sensor, public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_oem_clamp(oem_clamp::OemClamp *oem_clamp) { this->oem_clamp_ = oem_clamp; }
  void set_type(SensorType type);
 protected:
   void update_current(const OemClampData& data);
   void update_voltage(const OemClampData& data);
   void update_apparent_power(const OemClampData& data);
   void update_real_power(const OemClampData& data);

  oem_clamp::OemClamp *oem_clamp_;
  SensorType type_;
};

}  // namespace oem_clamp
}  // namespace esphome