#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/core/entity_base.h"
#include <functional>

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

private:
  std::string build_member_topic(const std::string &member) const;

  MylifeClientComponent *client_;
  std::string id_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
