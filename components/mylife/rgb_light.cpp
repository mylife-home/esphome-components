#include "rgb_light.h"

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
  .id = "light.rgb",
  .metadata = "{\"name\":\"rgb\",\"module\":\"light\",\"usage\":\"actuator\",\"version\":\"" MYLIFE_VERSION "\",\"config\":{},\"members\":{\"active\":{\"memberType\":\"state\",\"valueType\":\"bool\"},\"color\":{\"memberType\":\"state\",\"valueType\":\"range[0;16777215]\"},\"setActive\":{\"memberType\":\"action\",\"valueType\":\"bool\"},\"setColor\":{\"memberType\":\"action\",\"valueType\":\"range[0;16777215]\"}}}"
};

MylifeRgbLight::MylifeRgbLight(MylifeClientComponent *client, light::LightState *target)
  : MylifeController(client, target)
  , light_(target) {

  subscribe_action("setActive", [this](const std::string &buffer) {
    this->on_set_active(Encoding::read_bool(buffer));
  });

  subscribe_action("setColor", [this](const std::string &buffer) {
    this->on_set_color(Encoding::read_uint32(buffer));
  });

  light_->add_new_remote_values_callback([this]() {
    this->on_light_change();
  });
}

const PluginDefinition *MylifeRgbLight::get_plugin_metadata() const {
  return &definition;
}

void MylifeRgbLight::publish_states() {
  this->publish_states_(true);
}

void MylifeRgbLight::on_set_active(bool value) {
  auto call = this->light_->make_call();
  call.set_state(value);
  call.perform();
}

void MylifeRgbLight::on_set_color(uint32_t value) {
  color_encoding color_enc;
  color_enc.color = value;

  auto call = this->light_->make_call();

  call.set_brightness(1);

  call.set_red(static_cast<float>(color_enc.red) / 255);
  call.set_green(static_cast<float>(color_enc.green) / 255);
  call.set_blue(static_cast<float>(color_enc.blue) / 255);

  call.perform();
}

void MylifeRgbLight::on_light_change() {
  this->publish_states_(false);
}

void MylifeRgbLight::publish_states_(bool force) {
  // cf. light_json_schema.cpp
  const auto &values = this->light_->remote_values;

  auto active = values.get_state() != 0.0f;

  color_encoding color_enc;
  color_enc.red = uint8_t(values.get_color_brightness() * values.get_red() * 255);
  color_enc.green = uint8_t(values.get_color_brightness() * values.get_green() * 255);
  color_enc.blue = uint8_t(values.get_color_brightness() * values.get_blue() * 255);
  auto color = color_enc.color;

  if (force || active != active_) {
    active_ = active;
    this->publish_state("active", Encoding::write_bool(active_));
  }

  if (force || color != color_) {
    color_ = color;
    this->publish_state("color", Encoding::write_uint32(color_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
