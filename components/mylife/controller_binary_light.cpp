#include "controller_binary_light.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

namespace {
  union color_encoding {
    uint32_t color;
    struct {
      uint8_t red;
      uint8_t blue;
      uint8_t green;
    };
  };
}

static PluginDefinition definition {
  .id = "light.binary",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "binary",
  "module": "light",
  "usage": "actuator",
  "version": "{{version}}",
  "config": {},
  "members": {
    "active": { "memberType": "state", "valueType": "bool" },
    "setActive": { "memberType": "action", "valueType": "bool" }
  }
}
)EOF")
};

MylifeBinaryLight::MylifeBinaryLight(MylifeClientComponent *client, light::LightState *target)
  : MylifeController(client, target)
  , light_(target) {

  subscribe_action("setActive", [this](const std::string &buffer) {
    this->on_set_active(Encoding::read_bool(buffer));
  });

  light_->add_new_remote_values_callback([this]() {
    this->on_light_change();
  });
}

const PluginDefinition *MylifeBinaryLight::get_plugin_metadata() const {
  return &definition;
}

void MylifeBinaryLight::publish_states() {
  this->publish_states_(true);
}

void MylifeBinaryLight::on_set_active(bool value) {
  auto call = this->light_->make_call();
  call.set_state(value);
  call.perform();
}

void MylifeBinaryLight::on_light_change() {
  this->publish_states_(false);
}

void MylifeBinaryLight::publish_states_(bool force) {
  // cf. light_json_schema.cpp
  const auto &values = this->light_->remote_values;

  auto active = values.get_state() != 0.0f;

  if (force || active != active_) {
    active_ = active;
    this->publish_state("active", Encoding::write_bool(active_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
