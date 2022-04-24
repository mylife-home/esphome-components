#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MQTT_STUB

#include "esphome/core/log.h"
#include "mqtt_backend.h"

// Override this only to keep its logging capabilities

namespace esphome {
namespace mqtt {

class MQTTConfig {
 public:
  MQTTConfig();

  /** Set the topic prefix that will be prepended to all topics together with "/". This will, in most cases,
   * be the name of your Application.
   *
   * For example, if "livingroom" is passed to this method, all state topics will, by default, look like
   * "livingroom/.../state"
   *
   * @param topic_prefix The topic prefix. The last "/" is appended automatically.
   */
  void set_topic_prefix(const std::string &topic_prefix);
  /// Get the topic prefix of this device, using default if necessary
  const std::string &get_topic_prefix() const;

  /// Manually set the topic used for logging.
  void set_log_message_template(MQTTMessage &&message);
  void set_log_level(int level);
  /// Get the topic used for logging. Defaults to "<topic_prefix>/debug" and the value is cached for speed.
  void disable_log_message();
  bool is_log_message_enabled() const;

  int get_log_level() const;
  const MQTTMessage &get_log_message() const;

 protected:

  std::string topic_prefix_{};
  MQTTMessage log_message_;
  int log_level_{ESPHOME_LOG_LEVEL};
};

extern MQTTConfig *global_mqtt_client;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace mqtt
}  // namespace esphome

#endif  // USE_MQTT_STUB
