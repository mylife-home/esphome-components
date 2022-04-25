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

    static int8_t read_int8(const std::string &buffer);
    static std::string write_int8(int8_t value);

    static uint32_t read_uint32(const std::string &buffer);
    static std::string write_uint32(uint32_t value);

    static int32_t read_int32(const std::string &buffer);
    static std::string write_int32(int32_t value);

    static float read_float(const std::string &buffer);
    static std::string write_float(float value);
  };

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
