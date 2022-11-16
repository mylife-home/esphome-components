#include "oem_clamp_sensor.h"

#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace oem_clamp {

static const char *const TAG = "oem_clamp_sensor";

void OemClampSensor::set_type(SensorType type) {
  this->type_ = type;

  // configure sensor
  this->set_state_class(sensor::StateClass::STATE_CLASS_MEASUREMENT);
  switch(this->type_) {
    case SensorType::CURRENT:
      this->set_device_class("current");
      this->set_unit_of_measurement("A");
      this->set_accuracy_decimals(2);
      break;

    case SensorType::VOLTAGE:
      this->set_device_class("voltage");
      this->set_unit_of_measurement("V");
      this->set_accuracy_decimals(0);
      break;

    case SensorType::APPARENT_POWER:
      this->set_device_class("apparent_power");
      this->set_unit_of_measurement("VA");
      this->set_accuracy_decimals(0);
      break;

    case SensorType::REAL_POWER:
      this->set_device_class("power");
      this->set_unit_of_measurement("W");
      this->set_accuracy_decimals(0);
      break;
  }
}

void OemClampSensor::setup() {
  // Setup the right callback
  switch(this->type_) {
    case SensorType::CURRENT:
      this->oem_clamp_->add_on_update_callback([this](const OemClampData &data) {
        this->update_current(data);
      });
      break;

    case SensorType::VOLTAGE:
      this->oem_clamp_->add_on_update_callback([this](const OemClampData &data) {
        this->update_voltage(data);
      });
      break;

    case SensorType::APPARENT_POWER:
      this->oem_clamp_->add_on_update_callback([this](const OemClampData &data) {
        this->update_apparent_power(data);
      });
      break;

    case SensorType::REAL_POWER:
      this->oem_clamp_->add_on_update_callback([this](const OemClampData &data) {
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

void OemClampSensor::dump_config() {
  LOG_SENSOR("", "OEM Clamp Sensor", this);
  ESP_LOGCONFIG(TAG, "  Clamp: %s", this->oem_clamp_->get_id().c_str());
  ESP_LOGCONFIG(TAG, "  Type: %s", type_to_str(this->type_));
}

void OemClampSensor::update_current(const OemClampData& data){
  this->publish_state(data.i_rms);
}

void OemClampSensor::update_voltage(const OemClampData& data){
  this->publish_state(data.v_rms);
}

void OemClampSensor::update_apparent_power(const OemClampData& data){
  this->publish_state(data.v_rms * data.i_rms);
}

void OemClampSensor::update_real_power(const OemClampData& data){
  this->publish_state(data.p_real);
}

}  // namespace oem_clamp
}  // namespace esphome