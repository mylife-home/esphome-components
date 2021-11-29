#include "sensor.h"

#ifdef USE_MYLIFE
#ifdef USE_SENSOR

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "sensor.numeric",
  .metadata = R"EOF(
{
  "name": "numeric",
  "module": "sensor",
  "usage": "sensor",
  "version": "MYLIFE_VERSION",
  "config": {},
  "members": {
    "value": { "memberType": "state", "valueType": "float" },
    "unitOfMeasurement": { "memberType": "state", "valueType": "text" },
    "accuracyDecimals": { "memberType": "state", "valueType": "range[-128;127]" },
    "deviceClass": { "memberType": "state", "valueType": "text" },
    "stateClass": { "memberType": "state", "valueType": "enum{none,measurement,total-increasing,unknown}" }
  }
}
)EOF"
};

MylifeSensor::MylifeSensor(MylifeClientComponent *client, sensor::Sensor *target)
  : MylifeController(client, target)
  , sensor_(target) {

  sensor_->add_on_state_callback([this](float) {
    this->on_sensor_change();
  });
}

const PluginDefinition *MylifeSensor::get_plugin_metadata() const {
  return &definition;
}

void MylifeSensor::publish_states() {
  this->publish_states_(true);
}

void MylifeSensor::on_sensor_change() {
  this->publish_states_(false);
}

static std::string get_state_class(sensor::StateClass class_) {
  switch(class_) {
  case sensor::StateClass::STATE_CLASS_NONE:
    return "none";
  case sensor::StateClass::STATE_CLASS_MEASUREMENT:
    return "measurement";
  case sensor::StateClass::STATE_CLASS_TOTAL_INCREASING:
    return "total-increasing";
  default:
    return "unknown";
  }
}

void MylifeSensor::publish_states_(bool force) {
  auto value = this->sensor_->get_state();

  if (force || value != value_) {
    value_ = value;
    this->publish_state("value", Encoding::write_float(value_));
  }

  if (force) {
    this->publish_state("unitOfMeasurement", Encoding::write_string(this->sensor_->get_unit_of_measurement()));
    this->publish_state("accuracyDecimals", Encoding::write_int8(this->sensor_->get_accuracy_decimals()));
    this->publish_state("deviceClass", Encoding::write_string(this->sensor_->get_device_class()));
    this->publish_state("stateClass", Encoding::write_string(get_state_class(this->sensor_->get_state_class())));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_SENSOR
#endif  // USE_MYLIFE
