#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

namespace esphome {
namespace mylife {

class MylifeLight {
public:
  MylifeLight(light::LightState *target);

};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
