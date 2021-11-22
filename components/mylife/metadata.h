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
};

class Metadata {
public:
  explicit Metadata(MylifeClientComponent *client);

  void build_plugins(const std::vector<std::unique_ptr<MylifeController>> &controllers);

private:
  void publish();

  MylifeClientComponent *client_;
  std::set<const PluginDefinition*> plugins_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
