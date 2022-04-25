#include "mqtt_config.h"

#ifdef USE_MQTT_STUB

namespace esphome {
namespace mqtt {

MQTTConfig::MQTTConfig() {
  global_mqtt_client = this;
}

// Setters
void MQTTConfig::disable_log_message() { this->log_message_.topic = ""; }
bool MQTTConfig::is_log_message_enabled() const { return !this->log_message_.topic.empty(); }
void MQTTConfig::set_log_level(int level) { this->log_level_ = level; }
void MQTTConfig::set_log_message_template(MQTTMessage &&message) { this->log_message_ = std::move(message); }
void MQTTConfig::set_topic_prefix(const std::string &topic_prefix) { this->topic_prefix_ = topic_prefix; }
const std::string &MQTTConfig::get_topic_prefix() const { return this->topic_prefix_; }

int MQTTConfig::get_log_level() const { return this->log_level_; }
const MQTTMessage &MQTTConfig::get_log_message() const { return this->log_message_; }

MQTTConfig *global_mqtt_client = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace mqtt
}  // namespace esphome

#endif  // USE_MQTT_STUB
