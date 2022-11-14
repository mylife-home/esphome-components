#pragma once


#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_SENSOR

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/mylife/client.h"

namespace esphome {
namespace energy_publisher {

class EnergyPublisher : public Component {
public:
  explicit EnergyPublisher() = default;
  virtual ~EnergyPublisher() = default;

  void set_sensor(sensor::Sensor * sensor) { this->sensor_ = sensor; }
  void set_mylife_client(mylife::MylifeClientComponent * client) { this->client_ = client; }

protected:
  void setup() override;

private:
  void do_publish();

  std::string id_;
  sensor::Sensor *sensor_{nullptr};
  mylife::MylifeClientComponent *client_{nullptr};
};

}  // namespace energy_publisher
}  // namespace esphome

#endif  // USE_SENSOR
#endif  // USE_MYLIFE
