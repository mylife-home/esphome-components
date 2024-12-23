#include <cinttypes>

#include "remote.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

static const char *const TAG = "somfy_rts";

namespace esphome {
namespace somfy_rts {

SomfyRtsRemote::SomfyRtsRemote(GPIOPin *pin, uint32_t address, uint repeat)
 : address_(address)
 , pin_(pin)
 , store_(nullptr)
 , rolling_code_(0)
 , repeat_(repeat) {
}

void SomfyRtsRemote::setup() {
  this->pin_->setup();
  this->pin_->pin_mode(gpio::FLAG_OUTPUT);

  // Random 32bit value; If this changes existing restore preferences are invalidated
  static const uint32_t RESTORE_STATE_VERSION = 0x4242BABEUL;

  uint32_t hash = fnv1_hash("somfy_rts_state_" + to_string(this->address_));
  this->store_ = global_preferences->make_preference<uint16_t>(hash ^ RESTORE_STATE_VERSION);
  if (this->store_.load(&this->rolling_code_)) {
    ESP_LOGD(TAG, "Restored rolling code: %04X", this->rolling_code_);
  } else {
    ESP_LOGD(TAG, "No rolling code found, starting from 0");
  }
}

void SomfyRtsRemote::dump_config() {
  ESP_LOGCONFIG(TAG, "SomfyRtsRemote:");
  ESP_LOGCONFIG(TAG, "  Address: %06X", this->address_);
  ESP_LOGCONFIG(TAG, "  Pin: %s", this->pin_->dump_summary().c_str());
  ESP_LOGCONFIG(TAG, "  Rolling code: %04X", this->rolling_code_);
  ESP_LOGCONFIG(TAG, "  Repeat: %" PRIu32, this->repeat_);
}

void SomfyRtsRemote::send_command(Command command) {
  uint16_t rolling_code = this->next_rolling_code();

  ESP_LOGD(TAG, "Sending command %s (%02X) with rolling code %04X", command_to_string(command), command, rolling_code);

  Frame frame(command, rolling_code, this->address_);

  // Encode the frame for sending
  uint8_t data[Frame::size];
  frame.to_bytes(data);

  for (uint32_t i = 0; i < this->repeat_; i++) {
    this->send_frame(data, i > 0);
  }
}

void SomfyRtsRemote::send_frame(const uint8_t *data, bool repeated) {
    // Width of the high part of a "wakeup pulse", in microseconds
    constexpr uint32_t wakeup_high = 10568;
    // Width of the low part of a "wakeup pulse", in microseconds
    constexpr uint32_t wakeup_low = 7072;
    // Width of the high part of a "preamble", in microseconds
    constexpr uint32_t preamble_high = 2585;
    // Width of the low part of a "preamble", in microseconds
    constexpr uint32_t preamble_low = 2436;
    // Width of the high part of a "software sync", in microseconds
    constexpr uint32_t sync_high = 4898;
    // Width of the low part of a "software sync", in microseconds
    constexpr uint32_t sync_low = 644;
    // Width of a data symbol (either high or low), in microseconds
    constexpr uint32_t symbol = 644;
    // Width of an inter-frame sleep (low), in microseconds
    constexpr uint32_t frame_sleep = 26838;

    constexpr uint8_t initial_preamble_count = 2;
    constexpr uint8_t repeat_preamble_count = 7;

  if (!repeated) {
      // Send the wakeup pulses
      this->send_pulse(true, wakeup_high);
      this->send_pulse(false, wakeup_low);
  }

  // Decide how many preamble pulses to send
  uint8_t preamble_count = repeated ? repeat_preamble_count : initial_preamble_count;

  // Send the preamble pulses
  for (uint8_t i = 0; i < preamble_count; i++) {
      this->send_pulse(true, preamble_high);
      this->send_pulse(false, preamble_low);
  }

  // Send software sync pulses
  this->send_pulse(true, sync_high);
  this->send_pulse(false, sync_low);

  // Send the data
  for (int i = 0; i < Frame::size; i++) {
      // MSB first
      for (int j = 7; j >= 0; j--) {
          // Grab the bit to send
          bool bit = data[i] & (1 << j);

          // Send the bit using manchester encoded pulses
          if (bit == 1) {
              this->send_pulse(false, symbol);
              this->send_pulse(true, symbol);
          } else {
              this->send_pulse(true, symbol);
              this->send_pulse(false, symbol);
          }
      }
  }

  // Send the inter-frame sleep pulse
  this->send_pulse(false, frame_sleep);
}

void SomfyRtsRemote::send_pulse(bool state, uint32_t micros) {
  this->pin_->digital_write(state);
  delayMicroseconds(micros);
}

uint16_t SomfyRtsRemote::next_rolling_code() {
  ++this->rolling_code_;
  this->store_.save(&this->rolling_code_);
  return this->rolling_code_;
}


}  // namespace somfy_rts
}  // namespace esphome
