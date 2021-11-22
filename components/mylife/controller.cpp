#include "controller.h"

#ifdef USE_MYLIFE

#include "client.h"
#include "esphome/components/json/json_util.h"
#include "esphome/core/application.h"
#include <algorithm>

namespace esphome {
namespace mylife {

MylifeController::MylifeController(MylifeClientComponent *client, EntityBase *target)
 : client_(client)
 , id_(target->get_object_id()) {

  std::replace(id_.begin(), id_.end(), '_', '-');

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->publish_metadata();
      this->publish_status();
    }
  });
}


const std::string &MylifeController::get_component_id() const {
  return id_;
}

void MylifeController::subscribe_action(const std::string &action, std::function<void(const std::string &buffer)> handler) {
  auto callback = [handler](const std::string &topic, const std::string &payload) {
    handler(payload);
  };

  auto topic = build_member_topic(action);
  client_->subscribe(topic, callback);
}

void MylifeController::publish_state(const std::string &state, const std::string &buffer) {
  auto topic = build_member_topic(state);
  client_->publish(topic, buffer, 0, true);
}

std::string MylifeController::build_member_topic(const std::string &member) const {
  return App.get_name() + "/components/" + this->get_component_id() + "/" + member;
}

void MylifeController::publish_metadata() {
  auto topic = App.get_name() + "/metadata/components/" + this->id_;

  auto builder = [this](JsonObject &root) {
    root["id"] = this->id_;
    root["plugin"] = this->get_plugin_metadata()->id;
  };

  size_t len;
  const char *payload = json::build_json(builder, &len);
  client_->publish(topic, payload, len, 0, true);
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
