#pragma once


#include "opcode.h"
#include "machine.h"

namespace esphome {
namespace fairy_lights {
namespace instructions {

bool execute(Machine &machine, OpCode op);

}  // namespace instructions
}  // namespace fairy_lights
}  // namespace esphome
