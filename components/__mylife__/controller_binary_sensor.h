#pragma once


#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_BINARY_SENSOR

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeBinarySensor : public MylifeController {
public:
  explicit MylifeBinarySensor(MylifeClientComponent *client, binary_sensor::BinarySensor *target);
  virtual ~MylifeBinarySensor() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_sensor_change();

  void publish_states_(bool force);

  bool value_;
  binary_sensor::BinarySensor *sensor_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_BINARY_SENSOR
#endif  // USE_MYLIFE
