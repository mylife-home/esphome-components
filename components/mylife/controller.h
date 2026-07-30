#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/core/entity_base.h"
#include <functional>
#include <set>

namespace esphome {
namespace mylife {

class MylifeClientComponent;
struct PluginDefinition;

class MylifeController {
public:
  explicit MylifeController(MylifeClientComponent *client, EntityBase *target);
  virtual ~MylifeController() = default;

  virtual const PluginDefinition *get_plugin_metadata() const = 0; // Note: same pointer = same plugin

  const std::string &get_component_id() const;

protected:
  void subscribe_action(const std::string &action, std::function<void(const std::string &buffer)> handler);
  void publish_state(const std::string &state, const std::string &buffer);

  virtual void publish_states() = 0;

  // Republish part of the Component API from MylifeClientComponent, the only component in the tree
  void set_interval(uint32_t interval, std::function<void()> &&f);
  void set_interval(const std::string &name, uint32_t interval, std::function<void()> &&f);
  bool cancel_interval(const std::string &name);

  void set_timeout(uint32_t timeout, std::function<void()> &&f);
  void set_timeout(const std::string &name, uint32_t timeout, std::function<void()> &&f);
  bool cancel_timeout(const std::string &name); 

  void defer(const std::string &name, std::function<void()> &&f);
  void defer(std::function<void()> &&f);
  
private:
  std::string build_member_topic(const std::string &member) const;
  const char *make_name(const std::string &name);

  MylifeClientComponent *client_;
  std::string id_;
  std::set<std::string> scheduler_names_; // no very elegant, but OK for the usage we have now
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
