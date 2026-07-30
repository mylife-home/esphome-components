#include "controller.h"

#ifdef USE_MYLIFE

#include "client.h"
#include "esphome/components/json/json_util.h"
#include "esphome/core/application.h"
#include <algorithm>

namespace esphome {
namespace mylife {

static std::string get_object_id(EntityBase *entity) {
  std::string buf;
  buf.resize(OBJECT_ID_MAX_LEN);
  auto written = entity->write_object_id_to(buf.data(), buf.size());
  buf.resize(written);
  return buf;
}

MylifeController::MylifeController(MylifeClientComponent *client, EntityBase *target)
 : client_(client)
 , id_(get_object_id(target)) {

  std::replace(id_.begin(), id_.end(), '_', '-');

  client_->add_on_online_callback([this](bool online) {
    if (online) {
        this->publish_states();
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
  return client_->build_topic({"components", this->get_component_id(), member});
}

void MylifeController::set_interval(uint32_t interval, std::function<void()> &&f) {
  this->client_->set_interval(interval, std::move(f));
}

void MylifeController::set_interval(const std::string &name, uint32_t interval, std::function<void()> &&f) {
  this->client_->set_interval(this->make_name(name), interval, std::move(f));
}

bool MylifeController::cancel_interval(const std::string &name) {
  return this->client_->cancel_interval(this->make_name(name));
}

void MylifeController::set_timeout(uint32_t timeout, std::function<void()> &&f) {
  this->client_->set_timeout(timeout, std::move(f));
}

void MylifeController::set_timeout(const std::string &name, uint32_t timeout, std::function<void()> &&f) {
  this->client_->set_timeout(this->make_name(name), timeout, std::move(f));
}

bool MylifeController::cancel_timeout(const std::string &name) {
  return this->client_->cancel_timeout(this->make_name(name));
}

void MylifeController::defer(const std::string &name, std::function<void()> &&f) {
  this->client_->defer(this->make_name(name), std::move(f));
}

void MylifeController::defer(std::function<void()> &&f) {
  this->client_->defer(std::move(f));
}

const char *MylifeController::make_name(const std::string &name) {
  auto value = this->id_ + ":" + name;
  auto [it, inserted] = this->scheduler_names_.insert(value);
  return it->c_str();
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
