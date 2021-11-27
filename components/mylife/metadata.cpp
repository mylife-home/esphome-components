#include "metadata.h"

#ifdef USE_MYLIFE

#include "controller.h"
#include "client.h"
#include "version.h"
#include "esphome/core/application.h"
#include "esphome/core/version.h"
#include "esphome/core/component.h"

#ifdef USE_WIFI
#include "esphome/components/wifi/wifi_component.h"
#endif

#include <string>

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

Metadata::Metadata(MylifeClientComponent *client)
 : client_(client) {

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->publish_plugins();
      this->publish_instance_info();
    }
  });
}

void Metadata::build_plugins(const std::vector<std::unique_ptr<MylifeController>> &controllers) {
  for (const auto &controller : controllers) {
    plugins_.emplace(controller->get_plugin_metadata());
  }
}

void Metadata::publish_plugins() {
  for (const auto *plugin : plugins_) {
    auto topic = client_->build_topic({"metadata/plugins", plugin->id});
    client_->publish(topic, plugin->metadata, 0, true);
  }
}

void Metadata::update() {
  update_uptime();
  publish_instance_info();
}

void Metadata::publish_instance_info() {
  if (!client_->is_connected()) {
    return;
  }

  auto topic = client_->build_topic("metadata/instance-info");

  auto generator = [this](JsonObject &root) {
    root["type"] = "core";
    root["hardware"] = ESPHOME_BOARD;
    root["hostname"] = App.get_name();

#ifdef USE_WIFI
    root["wifi-signal-strength"] = wifi::global_wifi_component->wifi_rssi();
#endif

    JsonArray &capabilities = root.createNestedArray("capabilities");
    capabilities.add("components-manager");

    const auto uptime = this->get_uptime();
    root["systemUptime"] = uptime;
    root["instanceUptime"] = uptime;

    JsonObject &versions = root.createNestedObject("versions");
    versions["esphome"] = ESPHOME_VERSION;
    versions["mylife"] = MYLIFE_VERSION;
    versions["build"] = App.get_compilation_time();
#ifdef ESPHOME_PROJECT_NAME
    versions["project"] = ESPHOME_PROJECT_NAME " " ESPHOME_PROJECT_VERSION;
#endif

  };

  size_t len;
  const char *message = json::build_json(generator, &len);

  client_->publish(topic, message, len, 0, true);
}

void Metadata::update_uptime() {
  const uint32_t ms = millis();
  const uint64_t ms_mask = (1ULL << 32) - 1ULL;
  const uint32_t last_ms = this->uptime_ & ms_mask;
  if (ms < last_ms) {
    this->uptime_ += ms_mask + 1ULL;
    ESP_LOGD(TAG, "Detected roll-over \xf0\x9f\xa6\x84");
  }
  this->uptime_ &= ~ms_mask;
  this->uptime_ |= ms;
}

float Metadata::get_uptime() const {
  // Do separate second and milliseconds conversion to avoid floating point division errors
  // Probably some IEEE standard already guarantees this division can be done without loss
  // of precision in a single division, but let's do it like this to be sure.
  const uint64_t seconds_int = this->uptime_ / 1000ULL;
  return  float(seconds_int) + (this->uptime_ % 1000ULL) / 1000.0f;
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
