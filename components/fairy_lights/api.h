#pragma once

#include <cstdint>
#include <vector>
#include <ostream>

#include "opcode.h"

namespace esphome {
namespace fairy_lights {

class Api {
public:
  int32_t rand(int32_t min, int32_t max) const;
  std::size_t len() const;

  bool get(std::size_t index, uint8_t *red, uint8_t *green, uint8_t *blue) const;
  bool set(std::size_t index, uint8_t red, uint8_t green, uint8_t blue) const;
};

}  // namespace fairy_lights
}  // namespace esphome
