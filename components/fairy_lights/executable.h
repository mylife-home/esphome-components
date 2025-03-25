#pragma once

#include <cstdint>
#include <vector>
#include <ostream>
#include <memory>
#include <optional>

#include "opcode.h"

namespace esphome {
namespace fairy_lights {

class Executable {
public:
  explicit Executable(uint32_t stack_size, uint32_t locals_size, std::vector<OpCode> code)
    : stack_size_(stack_size), locals_size_(locals_size), code_(std::move(code)) {
  }

  static std::optional<Executable> from_base64(const std::string &value);

  size_t stack_size() const {
    return this->stack_size_;
  }

  size_t locals_size() const {
    return this->locals_size_;
  }

  const std::vector<OpCode> &code() const {
    return this->code_;
  }

private:
  uint32_t stack_size_;
  uint32_t locals_size_;
  std::vector<OpCode> code_;
};

std::ostream& operator<< (std::ostream& stream, const Executable& executable);

}  // namespace fairy_lights
}  // namespace esphome
