#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_SENSOR

#include "esphome/components/sensor/sensor.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeSensor : public MylifeController {
public:
  explicit MylifeSensor(MylifeClientComponent *client, sensor::Sensor *target);
  virtual ~MylifeSensor() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_sensor_change();

  void publish_states_(bool force);

  float value_;
  sensor::Sensor *sensor_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_SENSOR
#endif  // USE_MYLIFE
