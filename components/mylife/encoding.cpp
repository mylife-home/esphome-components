#include "encoding.h"

#ifdef USE_MYLIFE

#include "esphome/core/log.h"

namespace esphome {
namespace mylife {

  static const char *const TAG = "mylife";

  static void wrong_buffer() {
    ESP_LOGE(TAG, "Encoding: Got wrong buffer!");
  }

  std::string Encoding::read_string(const std::string &buffer) {
    return buffer;
  }

  std::string Encoding::write_string(const std::string &value) {
    return value;
  }

  bool Encoding::read_bool(const std::string &buffer) {
    return !!read_uint8(buffer);
  }

  std::string Encoding::write_bool(bool value) {
    return write_uint8(value ? 1 : 0);
  }

  uint8_t Encoding::read_uint8(const std::string &buffer) {
    if (buffer.size() != 1) {
      wrong_buffer();
      return 0;
    }
    
    return static_cast<uint8_t>(buffer[0]);
  }

  std::string Encoding::write_uint8(uint8_t value) {
    return std::string(static_cast<char>(value), 1);
  }

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
