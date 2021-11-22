#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/core/entity_base.h"

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
  MylifeClientComponent *client() const;

  virtual void publish_status() = 0;

private:
  void publish_metadata();
  
  MylifeClientComponent *client_;
  std::string id_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
