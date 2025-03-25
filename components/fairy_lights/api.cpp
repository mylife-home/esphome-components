#include <cstdint>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/color.h"
#include "esphome/components/light/esp_range_view.h"

#include "api.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {

Api::Api(light::ESPRangeView view)
 : view(std::make_unique<light::ESPRangeView>(view)) {
}

Api::~Api() {
}

int32_t Api::rand(int32_t min, int32_t max) const {
  auto fmin = static_cast<float>(min);
  auto fmax = static_cast<float>(max);

  auto fres = random_float() * (fmax - fmin) + fmin;

  return static_cast<int32_t>(fres);
}

std::size_t Api::len() const {
  return this->view->size();
}

void Api::get(std::size_t index, uint8_t *red, uint8_t *green, uint8_t *blue) const {
  auto color = (*this->view)[index];

  *red = color.get_red();
  *green = color.get_green();
  *blue = color.get_blue();
}

void Api::set(std::size_t index, uint8_t red, uint8_t green, uint8_t blue) const {
  auto color = (*this->view)[index];

  color.set_red(red);
  color.set_green(green);
  color.set_blue(blue);

  // ESP_LOGD(TAG, "Set color %d to %d %d %d", index, red, green, blue);
}

}  // namespace fairy_lights
}  // namespace esphome
