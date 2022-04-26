#pragma once


#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_TEXT_SENSOR

#include "esphome/components/text_sensor/text_sensor.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeTextSensor : public MylifeController {
public:
  explicit MylifeTextSensor(MylifeClientComponent *client, text_sensor::TextSensor *target);
  virtual ~MylifeTextSensor() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_sensor_change();

  void publish_states_(bool force);

  std::string value_;
  text_sensor::TextSensor *sensor_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_TEXT_SENSOR
#endif  // USE_MYLIFE
