#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

namespace esphome {
namespace mylife {

inline float color_utof(uint8_t value) {
  return static_cast<float>(value) / 255;
}

inline uint8_t color_ftou(float value) {
  return static_cast<uint8_t>(value * 255);
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
