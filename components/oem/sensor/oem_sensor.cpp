#include "oem_sensor.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem {

static const char *const TAG = "oem_sensor";

void OemSensor::setup() {
  // Setup the right callback
  switch(this->type_) {
    case SensorType::CURRENT:
      this->computer_->add_on_update_callback([this](const OemComputerData &data) {
        this->update_current(data);
      });
      break;

    case SensorType::VOLTAGE:
      this->computer_->add_on_update_callback([this](const OemComputerData &data) {
        this->update_voltage(data);
      });
      break;

    case SensorType::APPARENT_POWER:
      this->computer_->add_on_update_callback([this](const OemComputerData &data) {
        this->update_apparent_power(data);
      });
      break;

    case SensorType::REAL_POWER:
      this->computer_->add_on_update_callback([this](const OemComputerData &data) {
        this->update_real_power(data);
      });
      break;
  }
}

static const char *type_to_str(SensorType type) {
  switch(type) {
    case SensorType::CURRENT:
      return "current";

    case SensorType::VOLTAGE:
      return "voltage";

    case SensorType::APPARENT_POWER:
      return "apparent_power";

    case SensorType::REAL_POWER:
      return "real_power";

    default: 
      return "<unknown>";
  }
}

void OemSensor::dump_config() {
  LOG_SENSOR("", "OEM Sensor", this);
  ESP_LOGCONFIG(TAG, "  Computer: %s", this->computer_->get_id().c_str());
  ESP_LOGCONFIG(TAG, "  Type: %s", type_to_str(this->type_));
}

void OemSensor::update_current(const OemComputerData& data){
  this->publish_state(data.i_rms);
}

void OemSensor::update_voltage(const OemComputerData& data){
  this->publish_state(data.v_rms);
}

void OemSensor::update_apparent_power(const OemComputerData& data){
  this->publish_state(data.v_rms * data.i_rms);
}

void OemSensor::update_real_power(const OemComputerData& data){
  this->publish_state(data.p_real);
}

}  // namespace oem
}  // namespace esphome