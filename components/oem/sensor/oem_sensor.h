#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"
#include "esphome/components/oem/oem_computer.h"

namespace esphome {
namespace oem {

enum SensorType { CURRENT = 0, VOLTAGE, APPARENT_POWER, REAL_POWER };

class OemSensor : public sensor::Sensor, public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_computer(oem::OemComputer *computer) { this->computer_ = computer; }
  void set_type(SensorType type);
 protected:
   void update_current(const OemComputerData& data);
   void update_voltage(const OemComputerData& data);
   void update_apparent_power(const OemComputerData& data);
   void update_real_power(const OemComputerData& data);

  oem::OemComputer *computer_;
  SensorType type_;
};

}  // namespace oem
}  // namespace esphome