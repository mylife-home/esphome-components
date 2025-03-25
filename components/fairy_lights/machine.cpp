#include <cstdint>
#include <vector>
#include <ostream>
#include <memory>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "machine.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {

Machine::Machine(std::vector<int32_t> locals, std::vector<int32_t> stack, std::vector<OpCode> instructions, std::shared_ptr<Api> api)
  : locals_(std::move(locals))
  , stack_(std::move(stack))
  , stack_index_(0)
  , instructions_(std::move(instructions))
  , instruction_index_(0)
  , api_(std::move(api))
  , wakeup_time_(millis()) {
}

std::unique_ptr<Machine> Machine::load_executable(Executable exec, std::shared_ptr<Api> api) {
  std::vector<int32_t> locals(exec.locals_size(), 0);

  return std::make_unique<Machine>(
    std::vector<int32_t>(exec.locals_size()),
    std::vector<int32_t>(exec.stack_size()),
    exec.code(),
    api
  );
}

bool Machine::get_local(std::size_t index, int32_t *value) const {
  if (index >= locals_.size()) {
    ESP_LOGE(TAG, "Invalid local index: %zu", index);
    return false;
  }

  *value = locals_[index];
  return true;
}

bool Machine::set_local(std::size_t index, int32_t value) {
  if (index >= locals_.size()) {
    ESP_LOGE(TAG, "Invalid local index: %zu", index);
    return false;
  }

  locals_[index] = value;
  return true;
}

bool Machine::push(int32_t value) {
  if (stack_index_ == stack_.size()) {
    ESP_LOGE(TAG, "Stack overflow");
    return false;
  }

  stack_[stack_index_] = value;
  stack_index_ += 1;

  return true;
}

bool Machine::pop(int32_t *value) {
  if (stack_index_ == 0) {
    ESP_LOGE(TAG, "Stack underflow");
    return false;
  }

  stack_index_ -= 1;
  *value = stack_[stack_index_];

  return true;
}

std::optional<OpCode> Machine::fetch_instruction() {
  if (instruction_index_ >= instructions_.size()) {
    ESP_LOGE(TAG, "Invalid instruction index: %zu", instruction_index_);
    return std::nullopt;
  }

  return instructions_[instruction_index_++];
}

bool Machine::jump(int32_t relative_offset) {
  // instruction_index points to the next instruction, but relative offset is relative to the current instruction
  int32_t new_index = instruction_index_ - 1 + relative_offset;
  if (new_index < 0 || new_index >= instructions_.size()) {
    ESP_LOGE(TAG, "Invalid jump target: %d", new_index);
    return false;
  }

  instruction_index_ = new_index;
  return true;
}

void Machine::sleep(uint32_t duration) {
  wakeup_time_ = millis() + duration;
}

bool Machine::sleeping() const {
  return wakeup_time_ > millis();
}

}  // namespace fairy_lights
}  // namespace esphome
