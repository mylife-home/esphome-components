#include "metadata.h"

#ifdef USE_MYLIFE

#include "controller.h"
#include "client.h"
#include "esphome/core/application.h"
#include <string>

namespace esphome {
namespace mylife {

Metadata::Metadata(MylifeClientComponent *client)
 : client_(client) {

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->publish();
    }
  });

}

void Metadata::build_plugins(const std::vector<std::unique_ptr<MylifeController>> &controllers) {
  for (const auto &controller : controllers) {
    plugins_.emplace(controller->get_plugin_metadata());
  }
}

void Metadata::publish() {
  for (const auto *plugin : plugins_) {
    auto topic = App.get_name() + "/metadata/plugins/" + plugin->id;
    client_->publish(topic, plugin->metadata, 0, true);
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
