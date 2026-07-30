#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_CLIMATE

#include "esphome/components/climate/climate.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeClimate : public MylifeController {
public:
  explicit MylifeClimate(MylifeClientComponent *client, climate::Climate *target);
  virtual ~MylifeClimate() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_set_target_temperature(float value);
  void on_set_mode(climate::ClimateMode mode);
  void on_set_fan_mode(climate::ClimateFanMode fan_mode);

  void debounced_call(std::function<void(climate::ClimateCall &)> &&f);

  void on_climate_change();

  void publish_states_(bool force);

  float target_temperature_;
  climate::ClimateMode mode_;
  climate::ClimateFanMode fan_mode_;
  climate::Climate *climate_;
  std::optional<climate::ClimateCall> call_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_CLIMATE
#endif  // USE_MYLIFE
