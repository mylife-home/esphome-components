#include "client.h"

#ifdef USE_MYLIFE

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"
#include <utility>
#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif
#include "lwip/err.h"
#include "lwip/dns.h"

#include "controller.h"
#include "controller_factory.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

static std::string build_online_topic();

MylifeClientComponent::MylifeClientComponent()
 : metadata_(this) {
  this->credentials_.client_id = App.get_name() + "-" + get_mac_address() + "-mylife";
}

void MylifeClientComponent::add_on_online_callback(std::function<void(bool)> &&callback) {
  this->online_callback_.add(std::move(callback));
}

// Connection
void MylifeClientComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Mylife...");

  controllers_ = MylifeControllerFactory::build(this);
  metadata_.build_plugins(controllers_);

  this->mqtt_client_.onMessage([this](char const *topic, char *payload, AsyncMqttClientMessageProperties properties,
                                      size_t len, size_t index, size_t total) {
    if (index == 0)
      this->payload_buffer_.reserve(total);

    // append new payload, may contain incomplete MQTT message
    this->payload_buffer_.append(payload, len);

    // MQTT fully received
    if (len + index == total) {
      this->on_message(topic, this->payload_buffer_);
      this->payload_buffer_.clear();
    }
  });
  this->mqtt_client_.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
    this->state_ = MQTT_CLIENT_DISCONNECTED;
    this->disconnect_reason_ = reason;
  });
  
#ifdef USE_LOGGER
  if (logger::global_logger != nullptr) {
    logger::global_logger->add_on_log_callback([this](int level, const char *tag, const char *message) {
      if (level <= this->log_level_ && this->is_connected()) {
        int qos = 0;
        bool retain = true;
        //this->publish(this->log_message_.topic, message, strlen(message), qos, retain);
      }
    });
  }
#endif

  this->last_connected_ = millis();
  this->start_dnslookup_();
}

void MylifeClientComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Mylife:");
  ESP_LOGCONFIG(TAG, "  Server Address: %s:%u (%s)", this->credentials_.address.c_str(), this->credentials_.port,
                this->ip_.str().c_str());
  ESP_LOGCONFIG(TAG, "  Username: " LOG_SECRET("'%s'"), this->credentials_.username.c_str());
  ESP_LOGCONFIG(TAG, "  Client ID: " LOG_SECRET("'%s'"), this->credentials_.client_id.c_str());
  ESP_LOGCONFIG(TAG, "  Components:");

  for (const auto &controller : controllers_) {
    ESP_LOGCONFIG(TAG, "    %s (plugin: %s)", controller->get_component_id().c_str(), controller->get_plugin_metadata()->id.c_str());
  }
}

bool MylifeClientComponent::can_proceed() { return this->is_connected(); }

void MylifeClientComponent::start_dnslookup_() {
  for (auto &subscription : this->subscriptions_) {
    subscription.subscribed = false;
    subscription.resubscribe_timeout = 0;
  }

  this->status_set_warning();
  this->dns_resolve_error_ = false;
  this->dns_resolved_ = false;
  ip_addr_t addr;
#ifdef USE_ESP32
  err_t err = dns_gethostbyname_addrtype(this->credentials_.address.c_str(), &addr,
                                         MylifeClientComponent::dns_found_callback, this, LWIP_DNS_ADDRTYPE_IPV4);
#endif
#ifdef USE_ESP8266
  err_t err = dns_gethostbyname(this->credentials_.address.c_str(), &addr,
                                esphome::mylife::MylifeClientComponent::dns_found_callback, this);
#endif
  switch (err) {
    case ERR_OK: {
      // Got IP immediately
      this->dns_resolved_ = true;
#ifdef USE_ESP32
      this->ip_ = addr.u_addr.ip4.addr;
#endif
#ifdef USE_ESP8266
      this->ip_ = addr.addr;
#endif
      this->start_connect_();
      return;
    }
    case ERR_INPROGRESS: {
      // wait for callback
      ESP_LOGD(TAG, "Resolving MQTT broker IP address...");
      break;
    }
    default:
    case ERR_ARG: {
      // error
#if defined(USE_ESP8266)
      ESP_LOGW(TAG, "Error resolving MQTT broker IP address: %ld", err);
#else
      ESP_LOGW(TAG, "Error resolving MQTT broker IP address: %d", err);
#endif
      break;
    }
  }

  this->state_ = MQTT_CLIENT_RESOLVING_ADDRESS;
  this->connect_begin_ = millis();
}

void MylifeClientComponent::check_dnslookup_() {
  if (!this->dns_resolved_ && millis() - this->connect_begin_ > 20000) {
    this->dns_resolve_error_ = true;
  }

  if (this->dns_resolve_error_) {
    ESP_LOGW(TAG, "Couldn't resolve IP address for '%s'!", this->credentials_.address.c_str());
    this->state_ = MQTT_CLIENT_DISCONNECTED;
    return;
  }

  if (!this->dns_resolved_) {
    return;
  }

  ESP_LOGD(TAG, "Resolved broker IP address to %s", this->ip_.str().c_str());
  this->start_connect_();
}

#if defined(USE_ESP8266) && LWIP_VERSION_MAJOR == 1
void MylifeClientComponent::dns_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg) {
#else
void MylifeClientComponent::dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
#endif
  auto *a_this = (MylifeClientComponent *) callback_arg;
  if (ipaddr == nullptr) {
    a_this->dns_resolve_error_ = true;
  } else {
#ifdef USE_ESP32
    a_this->ip_ = ipaddr->u_addr.ip4.addr;
#endif
#ifdef USE_ESP8266
    a_this->ip_ = ipaddr->addr;
#endif
    a_this->dns_resolved_ = true;
  }
}

void MylifeClientComponent::start_connect_() {
  if (!network::is_connected())
    return;

  ESP_LOGI(TAG, "Connecting to MQTT...");
  // Force disconnect first
  this->mqtt_client_.disconnect(true);

  this->mqtt_client_.setClientId(this->credentials_.client_id.c_str());
  const char *username = nullptr;
  if (!this->credentials_.username.empty())
    username = this->credentials_.username.c_str();
  const char *password = nullptr;
  if (!this->credentials_.password.empty())
    password = this->credentials_.password.c_str();

  this->mqtt_client_.setCredentials(username, password);

  this->mqtt_client_.setServer((uint32_t) this->ip_, this->credentials_.port);

  auto will_topic = build_online_topic();
  auto will_payload = Encoding::write_bool(false);
  auto will_qos = 0;
  auto will_retain = true;
  this->mqtt_client_.setWill(will_topic.c_str(), will_qos, will_retain, will_payload.c_str(), will_payload.length());

  this->mqtt_client_.connect();
  this->state_ = MQTT_CLIENT_CONNECTING;
  this->connect_begin_ = millis();
}

bool MylifeClientComponent::is_connected() {
  return this->state_ == MQTT_CLIENT_CONNECTED && this->mqtt_client_.connected();
}

void MylifeClientComponent::check_connected() {
  if (!this->mqtt_client_.connected()) {
    if (millis() - this->connect_begin_ > 60000) {
      this->state_ = MQTT_CLIENT_DISCONNECTED;
      this->start_dnslookup_();
    }
    return;
  }

  this->state_ = MQTT_CLIENT_CONNECTED;
  this->status_clear_warning();
  ESP_LOGI(TAG, "MQTT Connected!");
  // MQTT Client needs some time to be fully set up.
  delay(100);  // NOLINT

  this->publish_online(true);

  this->last_connected_ = millis();

  this->resubscribe_subscriptions_();

  online_callback_.call(true);
}

void MylifeClientComponent::check_disconnected() {
  if (this->mqtt_client_.connected()) {
    this->last_connected_ = millis();
    return;
  }

  this->state_ = MQTT_CLIENT_DISCONNECTED;
  ESP_LOGW(TAG, "Lost MQTT Client connection!");
  this->start_dnslookup_();
}

static void log_disconnect(AsyncMqttClientDisconnectReason reason) {
  const LogString *reason_s;
  switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
      reason_s = LOG_STR("TCP disconnected");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
      reason_s = LOG_STR("Unacceptable Protocol Version");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
      reason_s = LOG_STR("Identifier Rejected");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
      reason_s = LOG_STR("Server Unavailable");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
      reason_s = LOG_STR("Malformed Credentials");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
      reason_s = LOG_STR("Not Authorized");
      break;
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
      reason_s = LOG_STR("Not Enough Space");
      break;
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
      reason_s = LOG_STR("TLS Bad Fingerprint");
      break;
    default:
      reason_s = LOG_STR("Unknown");
      break;
  }
  if (!network::is_connected()) {
    reason_s = LOG_STR("WiFi disconnected");
  }
  ESP_LOGW(TAG, "MQTT Disconnected: %s.", LOG_STR_ARG(reason_s));
}

void MylifeClientComponent::loop() {
  if (this->disconnect_reason_.has_value()) {
    log_disconnect(*this->disconnect_reason_);
    this->disconnect_reason_.reset();
    online_callback_.call(false);
  }

  switch (this->state_) {
    case MQTT_CLIENT_DISCONNECTED:
      if (millis() - this->connect_begin_ > 5000) {
        this->start_dnslookup_();
      }
      break;
    case MQTT_CLIENT_RESOLVING_ADDRESS:
      this->check_dnslookup_();
      break;
    case MQTT_CLIENT_CONNECTING:
      this->check_connected();
      break;
    case MQTT_CLIENT_CONNECTED:
      this->check_disconnected();
      break;
  }

  if (millis() - this->last_connected_ > this->reboot_timeout_ && this->reboot_timeout_ != 0) {
    ESP_LOGE(TAG, "Can't connect to MQTT... Restarting...");
    App.reboot();
  }
}

float MylifeClientComponent::get_setup_priority() const { return setup_priority::AFTER_WIFI; }

std::string build_online_topic() {
  return App.get_name() + "/online";
}

void MylifeClientComponent::publish_online(bool online) {
  auto topic = build_online_topic();
  auto payload = Encoding::write_bool(online);
  this->publish(topic, payload, 0, true);
}

// Subscribe
bool MylifeClientComponent::subscribe_(const char *topic, uint8_t qos) {
  if (!this->is_connected())
    return false;

  uint16_t ret = this->mqtt_client_.subscribe(topic, qos);
  yield();

  if (ret != 0) {
    ESP_LOGV(TAG, "subscribe(topic='%s')", topic);
  } else {
    delay(5);
    ESP_LOGV(TAG, "Subscribe failed for topic='%s'. Will retry later.", topic);
    this->status_momentary_warning("subscribe", 1000);
  }
  return ret != 0;
}

void MylifeClientComponent::resubscribe_subscription_(Subscription *sub) {
  if (sub->subscribed)
    return;

  const uint32_t now = millis();
  bool do_resub = sub->resubscribe_timeout == 0 || now - sub->resubscribe_timeout > 1000;

  if (do_resub) {
    sub->subscribed = this->subscribe_(sub->topic.c_str(), sub->qos);
    sub->resubscribe_timeout = now;
  }
}

void MylifeClientComponent::resubscribe_subscriptions_() {
  for (auto &subscription : this->subscriptions_) {
    this->resubscribe_subscription_(&subscription);
  }
}

void MylifeClientComponent::subscribe(const std::string &topic, subscription_callback_t callback, uint8_t qos) {
  Subscription subscription{
      .topic = topic,
      .qos = qos,
      .callback = std::move(callback),
      .subscribed = false,
      .resubscribe_timeout = 0,
  };
  this->resubscribe_subscription_(&subscription);
  this->subscriptions_.push_back(subscription);
}

void MylifeClientComponent::unsubscribe(const std::string &topic) {
  uint16_t ret = this->mqtt_client_.unsubscribe(topic.c_str());
  yield();
  if (ret != 0) {
    ESP_LOGV(TAG, "unsubscribe(topic='%s')", topic.c_str());
  } else {
    delay(5);
    ESP_LOGV(TAG, "Unsubscribe failed for topic='%s'.", topic.c_str());
    this->status_momentary_warning("unsubscribe", 1000);
  }

  auto it = subscriptions_.begin();
  while (it != subscriptions_.end()) {
    if (it->topic == topic)
      it = subscriptions_.erase(it);
    else
      ++it;
  }
}

// Publish
bool MylifeClientComponent::publish(const std::string &topic, const std::string &payload, uint8_t qos, bool retain) {
  return this->publish(topic, payload.data(), payload.size(), qos, retain);
}

bool MylifeClientComponent::publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos,
                                  bool retain) {
  if (!this->is_connected()) {
    // critical components will re-transmit their messages
    return false;
  }
  bool logging_topic = topic == this->log_message_.topic;
  uint16_t ret = this->mqtt_client_.publish(topic.c_str(), qos, retain, payload, payload_length);
  delay(0);
  if (ret == 0 && !logging_topic && this->is_connected()) {
    delay(0);
    ret = this->mqtt_client_.publish(topic.c_str(), qos, retain, payload, payload_length);
    delay(0);
  }

  if (!logging_topic) {
    if (ret != 0) {
      ESP_LOGV(TAG, "Publish(topic='%s' payload='%s' retain=%d)", topic.c_str(), payload, retain);
    } else {
      ESP_LOGV(TAG, "Publish failed for topic='%s' (len=%u). will retry later..", topic.c_str(),
               payload_length);  // NOLINT
      this->status_momentary_warning("publish", 1000);
    }
  }
  return ret != 0;
}

bool MylifeClientComponent::publish(const Message &message) {
  return this->publish(message.topic, message.payload, message.qos, message.retain);
}

/** Check if the message topic matches the given subscription topic
 *
 * INFO: MQTT spec mandates that topics must not be empty and must be valid NULL-terminated UTF-8 strings.
 *
 * @param message The message topic that was received from the MQTT server. Note: this must not contain
 *                wildcard characters as mandated by the MQTT spec.
 * @param subscription The subscription topic we are matching against.
 * @param is_normal Is this a "normal" topic - Does the message topic not begin with a "$".
 * @param past_separator Are we past the first '/' topic separator.
 * @return true if the subscription topic matches the message topic, false otherwise.
 */
static bool topic_match(const char *message, const char *subscription, bool is_normal, bool past_separator) {
  // Reached end of both strings at the same time, this means we have a successful match
  if (*message == '\0' && *subscription == '\0')
    return true;

  // Either the message or the subscribe are at the end. This means they don't match.
  if (*message == '\0' || *subscription == '\0')
    return false;

  bool do_wildcards = is_normal || past_separator;

  if (*subscription == '+' && do_wildcards) {
    // single level wildcard
    // consume + from subscription
    subscription++;
    // consume everything from message until '/' found or end of string
    while (*message != '\0' && *message != '/') {
      message++;
    }
    // after this, both pointers will point to a '/' or to the end of the string

    return topic_match(message, subscription, is_normal, true);
  }

  if (*subscription == '#' && do_wildcards) {
    // multilevel wildcard - MQTT mandates that this must be at end of subscribe topic
    return true;
  }

  // this handles '/' and normal characters at the same time.
  if (*message != *subscription)
    return false;

  past_separator = past_separator || *subscription == '/';

  // consume characters
  subscription++;
  message++;

  return topic_match(message, subscription, is_normal, past_separator);
}

static bool topic_match(const char *message, const char *subscription) {
  return topic_match(message, subscription, *message != '\0' && *message != '$', false);
}

void MylifeClientComponent::on_message(const std::string &topic, const std::string &payload) {
#ifdef USE_ESP8266
  // on ESP8266, this is called in LWiP thread; some components do not like running
  // in an ISR.
  this->defer([this, topic, payload]() {
#endif
    for (auto &subscription : this->subscriptions_)
      if (topic_match(topic.c_str(), subscription.topic.c_str()))
        subscription.callback(topic, payload);
#ifdef USE_ESP8266
  });
#endif
}

// Setters
void MylifeClientComponent::set_reboot_timeout(uint32_t reboot_timeout) { this->reboot_timeout_ = reboot_timeout; }
void MylifeClientComponent::set_keep_alive(uint16_t keep_alive_s) { this->mqtt_client_.setKeepAlive(keep_alive_s); }

void MylifeClientComponent::on_shutdown() {
  yield();
  this->publish_online(false);
  yield();
  this->mqtt_client_.disconnect(true);
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
