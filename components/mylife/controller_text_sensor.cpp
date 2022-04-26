#include "controller_text_sensor.h"

#ifdef USE_MYLIFE
#ifdef USE_TEXT_SENSOR

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "sensor.text",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "text",
  "module": "sensor",
  "usage": "sensor",
  "version": "{{version}}",
  "config": {},
  "members": {
    "value": { "memberType": "state", "valueType": "text" }
  }
}
)EOF")
};

MylifeTextSensor::MylifeTextSensor(MylifeClientComponent *client, text_sensor::TextSensor *target)
  : MylifeController(client, target)
  , sensor_(target) {

  sensor_->add_on_state_callback([this](std::string) {
    this->on_sensor_change();
  });
}

const PluginDefinition *MylifeTextSensor::get_plugin_metadata() const {
  return &definition;
}

void MylifeTextSensor::publish_states() {
  this->publish_states_(true);
}

void MylifeTextSensor::on_sensor_change() {
  this->publish_states_(false);
}

void MylifeTextSensor::publish_states_(bool force) {
  auto value = this->sensor_->get_state();

  if (force || value != value_) {
    value_ = value;
    this->publish_state("value", Encoding::write_string(value_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_TEXT_SENSOR
#endif  // USE_MYLIFE
