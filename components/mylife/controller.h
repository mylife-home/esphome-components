#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

namespace esphome {
namespace mylife {

struct PluginDefinition {
  std::string id;
  std::string metadata;
};

class MylifeController {
public:
  explicit MylifeController(MylifeClientComponent *client);
  virtual ~MylifeController() = default;

  virtual const PluginDefinition *get_plugin_metadata() const = 0; // Note: same pointer = same plugin
  virtual const std::string &get_component_id() const = 0;

protected:
  MylifeClientComponent *client() const {
    return client_;
  }

private:
  void publish_metadata();

  MylifeClientComponent *client_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
