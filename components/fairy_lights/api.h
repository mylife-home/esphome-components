#pragma once

#include <cstdint>

#include "opcode.h"

namespace esphome {
namespace fairy_lights {

class Api {
public:
  int32_t rand(int32_t min, int32_t max) const;
  std::size_t len() const;

  void get(std::size_t index, uint8_t *red, uint8_t *green, uint8_t *blue) const;
  void set(std::size_t index, uint8_t red, uint8_t green, uint8_t blue) const;
};

}  // namespace fairy_lights
}  // namespace esphome
