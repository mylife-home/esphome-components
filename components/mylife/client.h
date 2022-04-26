#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"
#include "esphome/components/network/ip_address.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/ota/ota_component.h"
#if defined(USE_ESP_IDF)
#include "esphome/components/mqtt/mqtt_backend_idf.h"
#elif defined(USE_ARDUINO)
#include "esphome/components/mqtt/mqtt_backend_arduino.h"
#endif
#include "lwip/ip_addr.h"
#include <vector>
#include <memory>
#include <set>
#include "metadata.h"
#include "logger.h"
#include "rpc.h"

namespace esphome {
namespace mylife {

class MylifeController;
struct PluginDefinition;

/** Callback for MQTT subscriptions.
 *
 * First parameter is the topic, the second one is the payload.
 */
using subscription_callback_t = std::function<void(const std::string &, const std::string &)>;

/// internal struct for MQTT subscriptions.
struct Subscription {
  std::string topic;
  uint8_t qos;
  subscription_callback_t callback;
  bool subscribed;
  uint32_t resubscribe_timeout;
};

/// internal struct for MQTT credentials.
struct Credentials {
  std::string address;  ///< The address of the server without port number
  uint16_t port;        ///< The port number of the server.
  std::string username;
  std::string password;
  std::string client_id;  ///< The client ID. Will automatically be truncated to 23 characters.
};

enum MQTTClientState {
  MQTT_CLIENT_DISCONNECTED = 0,
  MQTT_CLIENT_RESOLVING_ADDRESS,
  MQTT_CLIENT_CONNECTING,
  MQTT_CLIENT_CLEANING,
  MQTT_CLIENT_CONNECTED,
};

class MylifeClientComponent : public Component {
 public:
  MylifeClientComponent();

  /** Add a callback to be notified of online changes.
   *
   * @param callback The void(bool) callback.
   */
  void add_on_online_callback(std::function<void(bool)> &&callback);

  /// Set the keep alive time in seconds, every 0.7*keep_alive a ping will be sent.
  void set_keep_alive(uint16_t keep_alive_s);

#if ASYNC_TCP_SSL_ENABLED
  /** Add a SSL fingerprint to use for TCP SSL connections to the MQTT broker.
   *
   * To use this feature you first have to globally enable the `ASYNC_TCP_SSL_ENABLED` define flag.
   * This function can be called multiple times and any certificate that matches any of the provided fingerprints
   * will match. Calling this method will also automatically disable all non-ssl connections.
   *
   * @warning This is *not* secure and *not* how SSL is usually done. You'll have to add
   *          a separate fingerprint for every certificate you use. Additionally, the hashing
   *          algorithm used here due to the constraints of the MCU, SHA1, is known to be insecure.
   *
   * @param fingerprint The SSL fingerprint as a 20 value long std::array.
   */
  void add_ssl_fingerprint(const std::array<uint8_t, SHA1_SIZE> &fingerprint);
#endif
#ifdef USE_ESP_IDF
  void set_ca_certificate(const char *cert) { this->mqtt_backend_.set_ca_certificate(cert); }
  void set_skip_cert_cn_check(bool skip_check) { this->mqtt_backend_.set_skip_cert_cn_check(skip_check); }
#endif

  /** Subscribe to an MQTT topic and call callback when a message is received.
   *
   * @param topic The topic. Wildcards are currently not supported.
   * @param callback The callback function.
   * @param qos The QoS of this subscription.
   */
  void subscribe(const std::string &topic, subscription_callback_t callback, uint8_t qos = 0);

  /** Unsubscribe from an MQTT topic.
   *
   * If multiple existing subscriptions to the same topic exist, all of them will be removed.
   *
   * @param topic The topic to unsubscribe from.
   * Must match the topic in the original subscribe call exactly.
   */
  void unsubscribe(const std::string &topic);

  /** Publish a mqtt::MQTTMessage
   *
   * @param message The message.
   */
  bool publish(const mqtt::MQTTMessage &message);

  /** Publish a MQTT message
   *
   * @param topic The topic.
   * @param payload The payload.
   * @param retain Whether to retain the message.
   */
  bool publish(const std::string &topic, const std::string &payload, uint8_t qos = 0, bool retain = false);

  bool publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos = 0,
               bool retain = false);

  /// Setup the MQTT client, registering a bunch of callbacks and attempting to connect.
  void setup() override;
  void dump_config() override;
  /// Reconnect if required
  void loop() override;
  /// MQTT client setup priority
  float get_setup_priority() const override;

  void on_message(const std::string &topic, const std::string &payload);

  bool can_proceed() override;

  void set_reboot_timeout(uint32_t reboot_timeout);

  bool can_send();
  bool is_connected();

  void on_shutdown() override;

  void set_broker_address(const std::string &address) { this->credentials_.address = address; }
  void set_broker_port(uint16_t port) { this->credentials_.port = port; }
  void set_username(const std::string &username) { this->credentials_.username = username; }
  void set_password(const std::string &password) { this->credentials_.password = password; }
  void set_client_id(const std::string &client_id) { this->credentials_.client_id = client_id; }

  std::string build_topic(const std::string &suffix) const;
  std::string build_topic(std::initializer_list<std::string> suffix) const;

 protected:
  /// Reconnect to the MQTT broker if not already connected.
  void start_connect_();
  void start_dnslookup_();
  void check_dnslookup_();
#if defined(USE_ESP8266) && LWIP_VERSION_MAJOR == 1
  static void dns_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg);
#else
  static void dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
#endif
  void check_connected();
  void check_cleaned();
  void check_disconnected();

  bool subscribe_(const char *topic, uint8_t qos);
  void resubscribe_subscription_(Subscription *sub);
  void resubscribe_subscriptions_();

  void publish_online(bool online);

  Credentials credentials_;
  /// The last will message. Disabled optional denotes it being default and
  /// an empty topic denotes the the feature being disabled.
  mqtt::MQTTMessage last_will_;
  /// The birth message (e.g. the message that's send on an established connection.
  /// See last_will_ for what different values denote.
  mqtt::MQTTMessage birth_message_;
  bool sent_birth_message_{false};
  mqtt::MQTTMessage shutdown_message_;
  std::string topic_prefix_{};
  mqtt::MQTTMessage log_message_;
  std::string payload_buffer_;
  int log_level_{ESPHOME_LOG_LEVEL};

  std::vector<Subscription> subscriptions_;
#if defined(USE_ESP_IDF)
  mqtt::MQTTBackendIDF mqtt_backend_;
#elif defined(USE_ARDUINO)
  mqtt::MQTTBackendArduino mqtt_backend_;
#endif

  MQTTClientState state_{MQTT_CLIENT_DISCONNECTED};
  network::IPAddress ip_;
  bool dns_resolved_{false};
  bool dns_resolve_error_{false};
  uint32_t reboot_timeout_{300000};
  uint32_t connect_begin_;
  uint32_t last_connected_{0};
  uint32_t start_clean_{0};
  optional<mqtt::MQTTClientDisconnectReason> disconnect_reason_{};
  CallbackManager<void(bool)> online_callback_{};

  std::vector<std::unique_ptr<MylifeController>> controllers_;
  Metadata metadata_;
  Logger logger_;
  Rpc rpc_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
