#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include <set>
#include <vector>
#include <memory>

namespace esphome {
namespace mylife {

class MylifeClientComponent;
class MylifeController;

struct PluginDefinition {
  std::string id;
  std::string metadata;

  static std::string make_meta(std::string template_string);
};

class Metadata {
public:
  explicit Metadata(MylifeClientComponent *client);

  void build_plugins(const std::vector<std::unique_ptr<MylifeController>> &controllers);
  void update();
private:
  void publish_plugins();
  void publish_instance_info();

  void update_uptime();
  float get_uptime() const;

  MylifeClientComponent *client_;
  std::set<const PluginDefinition*> plugins_;
  uint64_t uptime_{0};
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
