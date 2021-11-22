#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include <string>

namespace esphome {
namespace mylife {

  class Encoding {
  public:
    static std::string read_string(const std::string &buffer);
    static std::string write_string(const std::string &value);

    static bool read_bool(const std::string &buffer);
    static std::string write_bool(bool value);

    static uint8_t read_uint8(const std::string &buffer);
    static std::string write_uint8(uint8_t value);
  };

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
