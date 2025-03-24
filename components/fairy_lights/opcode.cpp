#include <cstring>

#include "opcode.h"

namespace esphome {
namespace fairy_lights {

int32_t OpCode::arg24() const {
  union {
    struct {
      uint32_t value24: 24;
      uint32_t padding: 8;
    };
    int32_t value;
  } trans;

  trans.value24 = this->arg24_.arg;
  trans.padding = 0;
  return (trans.value << 8) >> 8;
}


std::ostream& operator<< (std::ostream& stream, const OpCode& op) {
  switch (op.type()) {
    // Stack management
    case OpCodeType::PUSH_CONSTANT:
      stream << "PushConstant(" << op.arg24() << ")";
      break;
    case OpCodeType::PUSH_VARIABLE:
      stream << "PushVariable(" << static_cast<size_t>(op.arg8()) << ")";
      break;
    case OpCodeType::POP_VARIABLE:
      stream << "PopVariable(" << static_cast<size_t>(op.arg8()) << ")";
      break;
    case OpCodeType::POP:
      stream << "Pop";
      break;

    // Compare
    case OpCodeType::EQUAL:
      stream << "Equal";
      break;
    case OpCodeType::NOT_EQUAL:
      stream << "NotEqual";
      break;
    case OpCodeType::LESS:
      stream << "Less";
      break;
    case OpCodeType::LESS_EQUAL:
      stream << "LessEqual";
      break;
  
    // Logic
    case OpCodeType::AND:
      stream << "And";
      break;
    case OpCodeType::OR:
      stream << "Or";
      break;
    case OpCodeType::NOT:
      stream << "Not";
      break;
  
    // Jump
    case OpCodeType::JUMP:
      stream << "Jump(" << op.arg24() << ")";
      break;
    case OpCodeType::JUMP_IF:
      stream << "JumpIf(" << op.arg24() << ")";
      break;
  
    // Arithmetic
    case OpCodeType::ADD:
      stream << "Add";
      break;
    case OpCodeType::SUB:
      stream << "Sub";
      break;
    case OpCodeType::MUL:
      stream << "Mul";
      break;
    case OpCodeType::DIV:
      stream << "Div";
      break;
    case OpCodeType::POW:
      stream << "Pow";
      break;
    case OpCodeType::MOD:
      stream << "Mod";
      break;
    
    // API
    case OpCodeType::RAND:
      stream << "Rand";
      break;
    case OpCodeType::LEN:
      stream << "Len";
      break;
    case OpCodeType::GET_RED:
      stream << "GetRed";
      break;
    case OpCodeType::GET_GREEN:
      stream << "GetGreen";
      break;
    case OpCodeType::GET_BLUE:
      stream << "GetBlue";
      break;
    case OpCodeType::SET:
      stream << "Set";
      break;
    case OpCodeType::SLEEP:
      stream << "Sleep";
      break;
  }

  return stream;
}

}  // namespace fairy_lights
}  // namespace esphome
