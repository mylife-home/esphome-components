#include "energy_publisher.h"

#ifdef USE_MYLIFE
#ifdef USE_SENSOR

namespace esphome {
namespace energy_publisher {

static const char *const TAG = "energy_publisher";

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

static std::string state_class_to_str(sensor::StateClass class_) {
  switch(class_) {
  case sensor::StateClass::STATE_CLASS_NONE:
    return "none";
  case sensor::StateClass::STATE_CLASS_MEASUREMENT:
    return "measurement";
  case sensor::StateClass::STATE_CLASS_TOTAL_INCREASING:
    return "total-increasing";
  default:
    return "unknown";
  }
}

void EnergyPublisher::do_publish() {
  if (!client_->is_connected()) {
    return;
  }

  if (!this->sensor_->has_state()) {
    ESP_LOGW(TAG, "Sensor '%s' has no state, will not publish.", this->id_.c_str());
    return;
  }

  auto value = this->sensor_->get_state();
  if (std::isnan(value)) {
    ESP_LOGW(TAG, "Got NaN value for sensor '%s', will not publish.", this->id_.c_str());
    return;
  }

  auto topic = this->client_->build_topic("energy");

  auto payload = json::build_json([this, value](JsonObject root) {
    root["id"] = this->id_;

    // sensor metadata
    root["device_class"] = this->sensor_->get_device_class();
    root["state_class"] = state_class_to_str(this->sensor_->get_state_class());
    root["unit_of_measurement"] = this->sensor_->get_unit_of_measurement();
    root["accuracy_decimals"] = this->sensor_->get_accuracy_decimals();

    root["value"] = value;
  });

  this->client_->publish(topic, payload, 0, false);
}

}  // namespace energy_publisher
}  // namespace esphome

#endif  // USE_SENSOR
#endif  // USE_MYLIFE
