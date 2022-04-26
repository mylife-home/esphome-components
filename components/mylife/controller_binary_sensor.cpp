#include "controller_binary_sensor.h"

#ifdef USE_MYLIFE
#ifdef USE_BINARY_SENSOR

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "sensor.binary",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "binary",
  "module": "sensor",
  "usage": "sensor",
  "version": "{{version}}",
  "config": {},
  "members": {
    "value": { "memberType": "state", "valueType": "bool" }
  }
}
)EOF")
};

MylifeBinarySensor::MylifeBinarySensor(MylifeClientComponent *client, binary_sensor::BinarySensor *target)
  : MylifeController(client, target)
  , sensor_(target) {

  sensor_->add_on_state_callback([this](bool) {
    this->on_sensor_change();
  });
}

const PluginDefinition *MylifeBinarySensor::get_plugin_metadata() const {
  return &definition;
}

void MylifeBinarySensor::publish_states() {
  this->publish_states_(true);
}

void MylifeBinarySensor::on_sensor_change() {
  this->publish_states_(false);
}

void MylifeBinarySensor::publish_states_(bool force) {
  auto value = this->sensor_->state;

  if (force || value != value_) {
    value_ = value;
    this->publish_state("value", Encoding::write_bool(value_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_BINARY_SENSOR
#endif  // USE_MYLIFE
