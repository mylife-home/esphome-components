#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeRgbLight : public MylifeController {
public:
  explicit MylifeRgbLight(MylifeClientComponent *client, light::LightState *target);
  virtual ~MylifeRgbLight() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;
  virtual const std::string &get_component_id() const override;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
