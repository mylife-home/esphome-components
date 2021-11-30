#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_SWITCH

#include "esphome/components/switch/switch.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeSwitch : public MylifeController {
public:
  explicit MylifeSwitch(MylifeClientComponent *client, switch_::Switch *target);
  virtual ~MylifeSwitch() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_set_value(bool value);
  void on_switch_change();

  void publish_states_(bool force);

  bool value_;
  switch_::Switch *switch_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_SWITCH
#endif  // USE_MYLIFE
