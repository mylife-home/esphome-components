#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

#include "controller.h"

namespace esphome {
namespace pico_epanel {

class InternalTempSensor : public PollingComponent, public Parented<PicoEpanelController>, public sensor::Sensor {
 public:
  InternalTempSensor();

  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;
};

}  // namespace pico_epanel
}  // namespace esphome
