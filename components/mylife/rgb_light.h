#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "esphome/components/light/light_state.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeRgbLight : public MylifeController {
public:
  explicit MylifeRgbLight(MylifeClientComponent *client, light::LightState *target);
  virtual ~MylifeRgbLight() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_status() override;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
