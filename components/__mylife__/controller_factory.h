#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include <vector>
#include <memory>

namespace esphome {
namespace mylife {

class MylifeController;
class MylifeClientComponent;

class MylifeControllerFactory {
public:
  static std::vector<std::unique_ptr<MylifeController>> build(MylifeClientComponent *client);
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
