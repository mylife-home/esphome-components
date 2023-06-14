#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include <set>
#include <vector>
#include <memory>
#include <string>

namespace esphome {
namespace mylife {

class MylifeClientComponent;
class MylifeController;

struct PluginDefinition {
  std::string id;
  std::string metadata;

  static std::string make_meta(std::string template_string);
};

struct ComponentDefinition {
  const std::string &id;
  const PluginDefinition *plugin;
};

class Metadata {
public:
  explicit Metadata(MylifeClientComponent *client);

  void build(const std::vector<std::unique_ptr<MylifeController>> &controllers);
  void update();
private:
  void publish_plugins();
  void publish_components();
  void publish_instance_info();

  void update_uptime();
  float get_uptime() const;

  MylifeClientComponent *client_;
  std::set<const PluginDefinition*> plugins_;
  std::vector<ComponentDefinition> components_;
  uint64_t uptime_{0};
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
