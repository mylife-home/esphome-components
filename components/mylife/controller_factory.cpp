#include "controller_factory.h"

#ifdef USE_MYLIFE

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "controller.h"

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

std::vector<std::unique_ptr<MylifeController>> MylifeControllerFactory::build(MylifeClientComponent *client) {
  ESP_LOGI(TAG, "TODO Controller factory");

  return  {};
}


}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE