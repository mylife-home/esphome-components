#include <cstdint>
#include <vector>
#include <ostream>
#include <optional>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "executable.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {

class BufferReader {
public:
  explicit BufferReader(const std::vector<uint8_t> &buffer)
   : current_(buffer.begin()), end_(buffer.end()) {
  }

  template<typename T>
  T read() {
    T value;
    this->read(&value, sizeof(T));
    return value;
  }

  void read(void *data, size_t size) {
    std::copy(this->current_, this->current_ + size, static_cast<uint8_t *>(data));
    this->current_ += size;
  }

  bool is_end() const {
    return this->current_ == this->end_;
  }

private:
  std::vector<uint8_t>::const_iterator current_;
  std::vector<uint8_t>::const_iterator end_;
};

class Crc32 {
  public:
    explicit Crc32()
      : value_(0) {
    }
  
    void update(const uint8_t *data, size_t size) {
      size_t i = 0;
  
      while (i < size) {
          auto table_index = static_cast<size_t>(((this->value_ >> 24) ^ static_cast<uint32_t>(data[i])) & 0xFF);
          this->value_ = TABLE[table_index] ^ (this->value_ << 8);
          i += 1;
      }
    }
  
    uint32_t finalize() const {
      return this->value_ ^ 0xffffffff;
    }
  
  private: 
    static uint32_t crc32(uint32_t poly, uint32_t value) {
      value <<= 24;
  
      size_t i = 0;
      while (i < 8) {
          value = (value << 1) ^ (((value >> 31) & 1) * poly);
          i += 1;
      }
  
      return value;
    }
  
    static std::array<uint32_t, 256> crc32_table() {
      std::array<uint32_t, 256> table = {0};
  
      size_t i = 0;
      while (i < 256) {
          table[i] = crc32(POLY, static_cast<uint32_t>(i));
          i += 1;
      }
    
      return table;
    }
  
    uint32_t value_;
  
    static constexpr uint32_t POLY = 0x04c11db7;
    static const std::array<uint32_t, 256> TABLE;
  };
  
  const std::array<uint32_t, 256> Crc32::TABLE = crc32_table();

std::optional<Executable> Executable::from_base64(const std::string &value) {
  constexpr uint32_t MAGIC = 0x00BABE00;

  auto data = base64_decode(value);

  if (data.size() < 32 || data.size() % 4 != 0) {
    ESP_LOGE(TAG, "Wrong executable size %u", data.size());
    return std::nullopt;
  }

  auto reader = BufferReader(data);

  auto magic = reader.read<uint32_t>();
  if (magic != MAGIC) {
    ESP_LOGE(TAG, "Wrong magic %08X (expected %08X)", magic, MAGIC);
    return std::nullopt;
  }

  auto crc = reader.read<uint32_t>();
  auto computer = Crc32();
  computer.update(data.data() + 8, data.size() - 8);
  auto expected_crc = computer.finalize();
  if (crc != expected_crc) {
    ESP_LOGE(TAG, "Invalid CRC %08X (expected %08X)", crc, expected_crc);
    return std::nullopt;
  }

  auto stack_size = reader.read<uint32_t>();
  auto locals_size = reader.read<uint32_t>();

  auto code = std::vector<OpCode>();
  while (!reader.is_end()) {
    auto op = OpCode::from_raw(reader.read<uint32_t>());
    code.push_back(op);
  }

  return Executable(stack_size, locals_size, code);
}

std::ostream& operator<< (std::ostream& stream, const Executable& executable) {
  stream << "StackSize=" << executable.stack_size() << "\n";
  stream << "LocalsSize=" << executable.locals_size() << "\n";
  stream << "\n";

  for (const auto &op : executable.code()) {
    stream << op << "\n";
  }

  return stream;
}

}  // namespace fairy_lights
}  // namespace esphome

