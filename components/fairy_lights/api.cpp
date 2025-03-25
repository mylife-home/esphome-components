#include <cstdint>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/color.h"

#include "api.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {

int32_t Api::rand(int32_t min, int32_t max) const {
  auto fmin = static_cast<float>(min);
  auto fmax = static_cast<float>(max);

  auto fres = random_float() * (fmax - fmin) + fmin;

  return static_cast<int32_t>(fres);
}

std::size_t Api::len() const {
  return 100; // TODO
}

// TODO
static Color colors[100] = {Color::BLACK};

void Api::get(std::size_t index, uint8_t *red, uint8_t *green, uint8_t *blue) const {
  const auto &color = colors[index];

  *red = color.red;
  *green = color.green;
  *blue = color.blue;
}

void Api::set(std::size_t index, uint8_t red, uint8_t green, uint8_t blue) const {
  auto &color = colors[index];

  color.red = red;
  color.green = green;
  color.blue = blue;

  ESP_LOGD(TAG, "Set color %d to %d %d %d", index, red, green, blue);
}

}  // namespace fairy_lights
}  // namespace esphome
