#include "metadata.h"

#ifdef USE_MYLIFE

#include "controller.h"
#include "client.h"
#include "version.h"
#include "esphome/core/application.h"
#include "esphome/core/version.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

#ifdef USE_WIFI
#include "esphome/components/wifi/wifi_component.h"
#endif

#include <string>

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

static void replace_all(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty()) {
    return;
  }

  size_t start_pos = 0;
  
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

std::string PluginDefinition::make_meta(std::string template_string) {
  replace_all(template_string, "{{version}}", MYLIFE_VERSION);
  return template_string;
}

Metadata::Metadata(MylifeClientComponent *client)
 : client_(client) {

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->publish_plugins();
      this->publish_components();
      this->publish_instance_info();
    }
  });
}

void Metadata::build(const std::vector<std::unique_ptr<MylifeController>> &controllers) {
  for (const auto &controller : controllers) {
    const auto *plugin = controller->get_plugin_metadata();
    plugins_.emplace(plugin);
    components_.emplace_back(ComponentDefinition{ .id = controller->get_component_id(), .plugin = plugin });
  }
}

void Metadata::publish_plugins() {
  for (const auto *plugin : plugins_) {
    auto topic = client_->build_topic({"metadata/plugins", plugin->id});
    client_->publish(topic, plugin->metadata, 0, true);
  }
}

void Metadata::publish_components() {
  for (const auto &component : components_) {
    auto topic = client_->build_topic({"metadata/components", component.id});

    auto payload = json::build_json([&component](JsonObject root) {
      root["id"] = component.id;
      root["plugin"] = component.plugin->id;
    });

    client_->publish(topic, payload.data(), payload.size(), 0, true);
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

  auto message = json::build_json([this](JsonObject root) {
    root["type"] = "core";
    root["hostname"] = App.get_name();

    JsonObject hardware = root.createNestedObject("hardware");
#if defined(ESPHOME_VARIANT) // Only keep this one when released
    hardware["main"] = ESPHOME_VARIANT;
#elif defined(USE_ESP32)
    hardware["main"] = "ESP32";
#elif defined(USE_ESP8266)
    hardware["main"] = "ESP8266";
#else
    hardware["main"] = "<unknown>";
#endif
    hardware["board"] = ESPHOME_BOARD;

    JsonArray capabilities = root.createNestedArray("capabilities");
    capabilities.add("components-manager");
    capabilities.add("restart-api");

#ifdef USE_WIFI
    capabilities.add("wifi-client");

    JsonObject wifi = root.createNestedObject("wifi");
    wifi["rssi"] = wifi::global_wifi_component->wifi_rssi();
#endif

    const auto uptime = this->get_uptime();
    root["systemUptime"] = uptime;
    root["instanceUptime"] = uptime;

    JsonObject versions = root.createNestedObject("versions");
    versions["esphome"] = ESPHOME_VERSION;
    versions["mylife"] = MYLIFE_VERSION;
    versions["build"] = App.get_compilation_time();
#ifdef ESPHOME_PROJECT_NAME
    versions["project"] = ESPHOME_PROJECT_NAME " " ESPHOME_PROJECT_VERSION;
#endif

  });

  client_->publish(topic, message, 0, true);
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
