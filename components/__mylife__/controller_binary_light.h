#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "esphome/components/light/light_state.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeBinaryLight : public MylifeController {
public:
  explicit MylifeBinaryLight(MylifeClientComponent *client, light::LightState *target);
  virtual ~MylifeBinaryLight() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_set_active(bool value);
  void on_light_change();

  void publish_states_(bool force);

  bool active_;
  light::LightState *light_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
