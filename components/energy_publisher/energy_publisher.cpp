#include "energy_publisher.h"

#ifdef USE_MYLIFE
#ifdef USE_SENSOR

namespace esphome {
namespace energy_publisher {

void EnergyPublisher::setup() {
  this->id_ = this->sensor_->get_object_id();
  std::replace(this->id_.begin(), this->id_.end(), '_', '-');

  this->sensor_->add_on_state_callback([this](float) {
    this->do_publish();
  });

  this->client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->do_publish();
    }
  });

  this->do_publish();
}

void EnergyPublisher::do_publish() {
  auto value = this->sensor_->get_state();
  int8_t accuracy = this->sensor_->get_accuracy_decimals();
  auto topic = this->client_->build_topic({"energy", this->id_});
  this->client_->publish(topic, value_accuracy_to_string(value, accuracy), 0, false);
}

}  // namespace energy_publisher
}  // namespace esphome

#endif  // USE_SENSOR
#endif  // USE_MYLIFE
