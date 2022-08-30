#include "internal_temp_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace pico_epanel {

static const char *const TAG = "pico_epanel.internal_temp_sensor";

InternalTempSensor::InternalTempSensor() {}

float InternalTempSensor::get_setup_priority() const { return setup_priority::DATA; }

void InternalTempSensor::dump_config() {
  LOG_SENSOR("", "Pico Epanel internal temp sensor", this);
  LOG_UPDATE_INTERVAL(this);
}

void InternalTempSensor::update() { 
  auto temp = this->parent_->read_internal_temp();
  this->publish_state(temp);
}

}  // namespace pico_epanel
}  // namespace esphome
