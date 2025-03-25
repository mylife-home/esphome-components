#include <cstdint>
#include <vector>
#include <ostream>
#include <cmath>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "instructions.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {
namespace instructions {

class Instructions {
public:
  explicit Instructions(Machine &pmachine, OpCode pop)
  : machine(pmachine)
  , op(pop) {    
  }

  bool execute() {
    switch (op.type()) {
      // Stack management
      case OpCodeType::PUSH_CONSTANT:
        return push_constant();
      case OpCodeType::PUSH_VARIABLE:
        return push_variable();
      case OpCodeType::POP_VARIABLE:
        return pop_variable();
      case OpCodeType::POP:
        return pop();
    
      // Compare
      case OpCodeType::EQUAL:
        return comparer(std::equal_to<int32_t>());
      case OpCodeType::NOT_EQUAL:
        return comparer(std::not_equal_to<int32_t>());
      case OpCodeType::LESS:
        return comparer(std::less<int32_t>());
      case OpCodeType::LESS_EQUAL:
        return comparer(std::less_equal<int32_t>());

      // Logic
      case OpCodeType::AND:
        return logic(std::logical_and<bool>());
      case OpCodeType::OR:
        return logic(std::logical_or<bool>());
      case OpCodeType::NOT:
        return not_();

      // Jump
      case OpCodeType::JUMP:
        return jump();
      case OpCodeType::JUMP_IF:
        return jump_if();

      // Arithmetic
      case OpCodeType::ADD:
        return arithmetic(std::plus<int32_t>());
      case OpCodeType::SUB:
        return arithmetic(std::minus<int32_t>());
      case OpCodeType::MUL:
        return arithmetic(std::multiplies<int32_t>());
      case OpCodeType::DIV:
        return arithmetic(std::divides<int32_t>());
      case OpCodeType::POW:
        return pow();
      case OpCodeType::MOD:
        return arithmetic(std::modulus<int32_t>());

      // API
      case OpCodeType::RAND:
        return rand();
      case OpCodeType::LEN:
        return len();
      case OpCodeType::GET_RED:
        return get_red();
      case OpCodeType::GET_GREEN:
        return get_green();
      case OpCodeType::GET_BLUE:
        return get_blue();
      case OpCodeType::SET:
        return set();
      case OpCodeType::SLEEP:
        return sleep();

      default:
        ESP_LOGE(TAG, "Runtime error: Unknown opcode");
        return false;
      }
    }

private:

  bool push_constant() {
    auto value = op.arg24();

    return machine.push(value);
  }

  bool push_variable() {
    auto index = op.arg8();

    int32_t value;
    return machine.get_local(index, &value) && machine.push(value);
  }

  bool pop_variable() {
    auto index = op.arg8();

    int32_t value;
    return machine.pop(&value) && machine.set_local(index, value);
  }

  bool pop() {
    int32_t value;
    return machine.pop(&value);
  }

  template<typename Op>// Op : fn(i32, i32) -> bool
  bool comparer(Op operation) {
    int32_t op1, op2;

    if (!machine.pop(&op2) || !machine.pop(&op1)) {
      return false;
    }

    int32_t result = operation(op1, op2) ? 1 : 0;

    return machine.push(result);
  }

  template<typename Op>// Op : fn(bool, bool) -> bool
  bool logic(Op op) {
    int32_t op1, op2;

    if (!machine.pop(&op2) || !machine.pop(&op1)) {
      return false;
    }

    int32_t result = op(op1 != 0, op2 != 0) ? 1 : 0;

    return machine.push(result);
  }

  bool not_() {
    int32_t op;

    if (!machine.pop(&op)) {
      return false;
    }

    int32_t result = op != 0 ? 0 : 1;

    return machine.push(result);
  }

  bool jump() {
    int32_t relative_offset = op.arg24();

    return machine.jump(relative_offset);
  }

  bool jump_if() {
    int32_t relative_offset = op.arg24();
    int32_t condition;

    if (!machine.pop(&condition)) {
      return false;
    }

    if (condition != 0) {
      return machine.jump(relative_offset);
    }

    return true;
  }

  template<typename Op>// Op : fn(i32, i32) -> i32
  bool arithmetic(Op op) {
    int32_t op1, op2;

    if (!machine.pop(&op2) || !machine.pop(&op1)) {
      return false;
    }

    int32_t result = op(op1, op2);

    return machine.push(result);
  }

  bool pow() {
    int32_t op1, op2;

    if (!machine.pop(&op2) || !machine.pop(&op1)) {
      return false;
    }

    if (op2 < 0) {
      ESP_LOGE(TAG, "Runtime error: Exponent must be non-negative");
      return false;
    }

    int32_t result = std::pow(op1, op2);

    return machine.push(result);
  }

  bool rand() {
    int32_t min, max;

    if (!machine.pop(&max) || !machine.pop(&min)) {
      return false;
    }

    int32_t result = machine.external_api().rand(min, max);

    return machine.push(result);
  }

  bool len() {
    int32_t result = static_cast<int32_t>(machine.external_api().len());

    return machine.push(result);
  }

  bool get_red() {
    int32_t red;

    if (!get_color(&red, nullptr, nullptr)) {
      return false;
    }

    return machine.push(red);
  }

  bool get_green() {
    int32_t green;

    if (!get_color(nullptr, &green, nullptr)) {
      return false;
    }

    return machine.push(green);
  }

  bool get_blue() {
    int32_t blue;

    if (!get_color(nullptr, nullptr, &blue)) {
      return false;
    }

    return machine.push(blue);
  }

  bool get_color(int32_t *pred, int32_t *pgreen, int32_t *pblue) {
    int32_t index;
    uint8_t red, green, blue;

    if (!machine.pop(&index)) {
      return false;
    }

    if (index < 0) {
      ESP_LOGE(TAG, "Runtime error: Index must be non-negative (get_color)");
      return false;
    }

    if (!machine.external_api().get(static_cast<std::size_t>(index), &red, &green, &blue)) {
      return false;
    }

    if (pred) {
      *pred = static_cast<int32_t>(red);
    }

    if (pgreen) {
      *pgreen = static_cast<int32_t>(green);
    }

    if (pblue) {
      *pblue = static_cast<int32_t>(blue);
    }

    return true;
  }

  bool set() {
    int32_t index;
    int32_t red, green, blue;

    if (!machine.pop(&blue)) {
      return false;
    }

    if (!machine.pop(&green)) {
      return false;
    }

    if (!machine.pop(&red)) {
      return false;
    }

    if (!machine.pop(&index)) {
      return false;
    }

    if (index < 0) {
        ESP_LOGE(TAG, "Runtime error: Index must be non-negative (set)");
        return false;
    }

    if (red < 0 || red > 255) {
        ESP_LOGE(TAG, "Runtime error: Red must be in the range 0-255 (set)");
        return false;
    }

    if (green < 0 || green > 255) {
        ESP_LOGE(TAG, "Runtime error: Red must be in the range 0-255 (set)");
        return false;
    }

    if (blue < 0 || blue > 255) {
        ESP_LOGE(TAG, "Runtime error: Red must be in the range 0-255 (set)");
        return false;
    }

    return machine.external_api().set(
      static_cast<std::size_t>(index),
      static_cast<uint8_t>(red),
      static_cast<uint8_t>(green),
      static_cast<uint8_t>(blue)
    );
  }

  bool sleep() {
    int32_t duration;

    if (!machine.pop(&duration)) {
      return false;
    }

    if (duration < 0) {
      ESP_LOGE(TAG, "Runtime error: Duration must be non-negative");
      return false;
    }

    machine.sleep(static_cast<uint32_t>(static_cast<uint32_t>(duration)));

    return true;
  }

  Machine &machine;
  OpCode op;
};

bool execute(Machine &machine, OpCode op) {
  return Instructions(machine, op).execute();
}

}  // namespace instructions
}  // namespace fairy_lights
}  // namespace esphome
