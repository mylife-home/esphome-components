#include "controller_monochromatic_light.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"
#include "color_utils.h"

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
  .id = "light.monochromatic",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "monochromatic",
  "module": "light",
  "usage": "actuator",
  "version": "{{version}}",
  "config": {},
  "members": {
    "active": { "memberType": "state", "valueType": "bool" },
    "brightness": { "memberType": "state", "valueType": "range[0;255]" },
    "setActive": { "memberType": "action", "valueType": "bool" },
    "setBrightness": { "memberType": "action", "valueType": "range[0;255]" }
  }
}
)EOF")
};

MylifeMonochromaticLight::MylifeMonochromaticLight(MylifeClientComponent *client, light::LightState *target)
  : MylifeController(client, target)
  , light_(target) {

  subscribe_action("setActive", [this](const std::string &buffer) {
    this->on_set_active(Encoding::read_bool(buffer));
  });

  subscribe_action("setBrightness", [this](const std::string &buffer) {
    this->on_set_brightness(Encoding::read_uint8(buffer));
  });

  light_->add_new_remote_values_callback([this]() {
    this->on_light_change();
  });
}

const PluginDefinition *MylifeMonochromaticLight::get_plugin_metadata() const {
  return &definition;
}

void MylifeMonochromaticLight::publish_states() {
  this->publish_states_(true);
}

void MylifeMonochromaticLight::on_set_active(bool value) {
  auto call = this->light_->make_call();
  call.set_state(value);
  call.perform();
}

void MylifeMonochromaticLight::on_set_brightness(uint8_t value) {
  auto call = this->light_->make_call();
  call.set_brightness(color_utof(value));
  call.perform();
}

void MylifeMonochromaticLight::on_light_change() {
  this->publish_states_(false);
}

void MylifeMonochromaticLight::publish_states_(bool force) {
  // cf. light_json_schema.cpp
  const auto &values = this->light_->remote_values;

  auto active = values.get_state() != 0.0f;
  auto brightness = color_ftou(values.get_brightness());

  if (force || active != active_) {
    active_ = active;
    this->publish_state("active", Encoding::write_bool(active_));
  }

  if (force || brightness != brightness_) {
    brightness_ = brightness;
    this->publish_state("brightness", Encoding::write_uint8(brightness_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
