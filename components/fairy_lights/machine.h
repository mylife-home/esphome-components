#pragma once

#include <cstdint>
#include <vector>
#include <ostream>
#include <memory>
#include <optional>

#include "esphome/core/hal.h"

#include "opcode.h"
#include "executable.h"
#include "api.h"

namespace esphome {
namespace fairy_lights {

class Machine {
public:
  explicit Machine(std::vector<int32_t> locals, std::vector<int32_t> stack, std::vector<OpCode> instructions, std::shared_ptr<Api> api);

  static std::unique_ptr<Machine> load_executable(Executable exec, std::shared_ptr<Api> api);

  bool get_local(std::size_t index, int32_t *value) const;
  bool set_local(std::size_t index, int32_t value);
  bool push(int32_t value);
  bool pop(int32_t *value);
  std::optional<OpCode> fetch_instruction();
  bool jump(int32_t relative_offset);
  void sleep(uint32_t duration);
  bool sleeping() const;

  const Api &external_api() const {
    return *api_;
  }

private:
  std::vector<int32_t> locals_;
  std::vector<int32_t> stack_;
  std::size_t stack_index_;
  std::vector<OpCode> instructions_;
  std::size_t instruction_index_;
  std::shared_ptr<Api> api_;
  uint32_t wakeup_time_;
};

}  // namespace fairy_lights
}  // namespace esphome
