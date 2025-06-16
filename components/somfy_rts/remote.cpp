#include <cinttypes>

#include "remote.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

static const char *const TAG = "somfy_rts";

namespace esphome {
namespace somfy_rts {

SomfyRtsRemote::SomfyRtsRemote(uint32_t address, uint repeat)
 : address_(address)
 , store_(nullptr)
 , rolling_code_(0)
 , repeat_(repeat) {
}

void SomfyRtsRemote::setup() {
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

  auto transmit = this->transmitter_->transmit();
  auto *tdata = transmit.get_data();
  tdata->set_carrier_frequency(0);

  for (uint32_t i = 0; i < this->repeat_; i++) {
    this->send_frame(tdata, data, i > 0);
  }

  transmit.perform();
}

void SomfyRtsRemote::send_frame(remote_base::RemoteTransmitData *tdata, const uint8_t *data, bool repeated) {
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
      tdata->mark(wakeup_high);
      tdata->space(wakeup_low);
  }

  // Decide how many preamble pulses to send
  uint8_t preamble_count = repeated ? repeat_preamble_count : initial_preamble_count;

  // Send the preamble pulses
  for (uint8_t i = 0; i < preamble_count; i++) {
      tdata->mark(preamble_high);
      tdata->space(preamble_low);
  }

  // Send software sync pulses
  tdata->mark(sync_high);
  tdata->space(sync_low);

  // Send the data
  for (int i = 0; i < Frame::size; i++) {
      // MSB first
      for (int j = 7; j >= 0; j--) {
          // Grab the bit to send
          bool bit = data[i] & (1 << j);

          // Send the bit using manchester encoded pulses
          if (bit == 1) {
              tdata->space(symbol);
              tdata->mark(symbol);
          } else {
              tdata->mark(symbol);
              tdata->space(symbol);
          }
      }
  }

  // Send the inter-frame sleep pulse
  tdata->space(frame_sleep);
}

uint16_t SomfyRtsRemote::next_rolling_code() {
  ++this->rolling_code_;
  this->store_.save(&this->rolling_code_);
  return this->rolling_code_;
}

}  // namespace somfy_rts
}  // namespace esphome
