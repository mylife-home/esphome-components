#include "command.h"

namespace esphome {
namespace somfy_rts {

const char *command_to_string(Command command) {
  switch (command) {
    case Command::MY: return "MY";
    case Command::UP: return "UP";
    case Command::MY_UP: return "MY_UP";
    case Command::DOWN: return "DOWN";
    case Command::MY_DOWN: return "MY_DOWN";
    case Command::UP_DOWN: return "UP_DOWN";
    case Command::MY_UP_DOWN: return "MY_UP_DOWN";
    case Command::PROG: return "PROG";
    case Command::SUN_FLAG: return "SUN_FLAG";
    case Command::FLAG: return "FLAG";
    default: return "UNKNOWN";
  }
}

}  // namespace somfy_rts
}  // namespace esphome