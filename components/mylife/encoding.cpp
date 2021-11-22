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

  namespace {
    template<typename ValueType>
    union exchange {
      ValueType value;
      char raw[sizeof(ValueType)];

      static std::string write(ValueType value) {
        exchange<ValueType> ex;
        ex.value = value;
        return std::string(ex.raw, sizeof(ValueType));
      }

      static ValueType read(const std::string &buffer) {
        if (buffer.size() != sizeof(ValueType)) {
          wrong_buffer();
          return 0;
        }

        exchange<ValueType> ex;
        memcpy(ex.raw, buffer.data(), sizeof(ValueType));
        return ex.value;
      }
    };
  }

  uint8_t Encoding::read_uint8(const std::string &buffer) {
    return exchange<uint8_t>::read(buffer);
  }

  std::string Encoding::write_uint8(uint8_t value) {
    return exchange<uint8_t>::write(value);
  }

  int8_t Encoding::read_int8(const std::string &buffer) {
    return exchange<int8_t>::read(buffer);
  }

  std::string Encoding::write_int8(int8_t value) {
    return exchange<int8_t>::write(value);
  }

  uint32_t Encoding::read_uint32(const std::string &buffer) {
    return exchange<uint32_t>::read(buffer);
  }

  std::string Encoding::write_uint32(uint32_t value) {
    return exchange<uint32_t>::write(value);
  }

  int32_t Encoding::read_int32(const std::string &buffer) {
    return exchange<int32_t>::read(buffer);
  }

  std::string Encoding::write_int32(int32_t value) {
    return exchange<int32_t>::write(value);
  }

  float Encoding::read_float(const std::string &buffer) {
    return exchange<float>::read(buffer);
  }

  std::string Encoding::write_float(float value) {
    return exchange<float>::write(value);
  }
}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
