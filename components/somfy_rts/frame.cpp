#include "frame.h"

namespace esphome {
namespace somfy_rts {


Frame::Frame(Command command, uint16_t rolling_code, uint32_t remote_address)
 : command_(command)
 , rolling_code_(rolling_code)
 , remote_address_(remote_address) {

  // Generate an encryption key that looks like one from a remote
  // All my remotes have the first nibble as 0xA, and the second
  // nibble as a number that increments for every button press.
  // Receivers don't seem to care about this.
  this->encryption_key_ = (this->rolling_code_ & 0xF) | 0xA0;
}

static void checksum(uint8_t *bytes) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < Frame::size; i++) {
      checksum = checksum ^ bytes[i] ^ (bytes[i] >> 4);
  }

  checksum = checksum & 0xF;
  bytes[1] |= checksum;
}

static void obfuscate(uint8_t *bytes) {
  // XOR between the byte to obfuscate and the previous obfuscated byte
  for (uint8_t i = 1; i < Frame::size; i++) {
      bytes[i] = bytes[i] ^ bytes[i - 1];
  }
}

void Frame::to_bytes(uint8_t *bytes) const {
  // Pack frame into bytes
  bytes[0] = this->encryption_key_;
  bytes[1] = static_cast<uint8_t>(this->command_) << 4;
  bytes[2] = this->rolling_code_ >> 8;
  bytes[3] = this->rolling_code_;
  bytes[4] = this->remote_address_;
  bytes[5] = this->remote_address_ >> 8;
  bytes[6] = this->remote_address_ >> 16;

  // Generate the checksum
  checksum(bytes);

  // Obfuscate
  obfuscate(bytes);
}


}  // namespace somfy_rts
}  // namespace esphome
