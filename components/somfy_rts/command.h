#pragma once

#include <cstdint>

namespace esphome {
namespace somfy_rts {

enum class Command : uint8_t {
  MY         = 0x1, ///< My/Stop button pressed
  UP         = 0x2, ///< Up button pressed
  MY_UP      = 0x3, ///< My and Up buttons both pressed
  DOWN       = 0x4, ///< Down button pressed
  MY_DOWN    = 0x5, ///< My and Down buttons both pressed
  UP_DOWN    = 0x6, ///< Up and Down buttons both pressed
  MY_UP_DOWN = 0x7, ///< My, Up and Down buttons all pressed
  PROG       = 0x8, ///< Prog button pressed
  SUN_FLAG   = 0x9, ///< Sun and Flag button pressed (Soliris)
  FLAG       = 0xA, ///< Flag button pressed (Soliris)
};

const char *command_to_string(Command command);

}  // namespace somfy_rts
}  // namespace esphome
