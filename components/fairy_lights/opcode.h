#pragma once

#include <cstdint>
#include <vector>
#include <ostream>

namespace esphome {
namespace fairy_lights {

enum class OpCodeType : uint8_t {
  // Stack management
  PUSH_CONSTANT, // value: i24
  PUSH_VARIABLE, // index: u8
  POP_VARIABLE, // index: u8
  POP,

  // Compare
  EQUAL,
  NOT_EQUAL,
  LESS,
  LESS_EQUAL,

  // Logic
  AND,
  OR,
  NOT,

  // Jump
  JUMP, // relative_offset: i24
  JUMP_IF, // relative_offset: i24

  // Arithmetic
  ADD,
  SUB,
  MUL,
  DIV,
  POW,
  MOD,
  
  // API
  RAND,
  LEN,
  GET_RED,
  GET_GREEN,
  GET_BLUE,
  SET,
  SLEEP,
};

class OpCode {
public:
  static OpCode from_raw(uint32_t raw) {
    OpCode op;
    op.raw_ = raw;
    return op;
  }

  OpCodeType type() const {
    return static_cast<OpCodeType>(this->arg0_.type_);
  }

  uint8_t arg8() const {
    return this->arg8_.arg;
  }

  int32_t arg24() const;

private:
  

  union {
    uint32_t raw_;

    struct {
      uint32_t type_:8;
    } arg0_;

    struct {
      uint32_t type:8;
      uint32_t arg:8;
    } arg8_;

    struct {
      uint32_t type:8;
      uint32_t arg:24;
    } arg24_;
  };
};

std::ostream& operator<< (std::ostream& stream, const OpCode& op);

}  // namespace fairy_lights
}  // namespace esphome
