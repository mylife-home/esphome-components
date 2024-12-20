#include "remote.h"
#include "esphome/core/log.h"

namespace esphome {
namespace somfy_rts {

SomfyRtsRemote::SomfyRtsRemote(InternalGPIOPin *pin, uint32_t address)
 : address_(address)
 , pin_(pin)
 , store_(nullptr)
 , rolling_code_(0) {
}

void SomfyRtsRemote::setup() {
  this->pin_->setup();
  this->pin_->pin_mode(gpio::FLAG_OUTPUT);

  // Random 32bit value; If this changes existing restore preferences are invalidated
  static const uint32_t RESTORE_STATE_VERSION = 0x4242BABEUL;

  uint32_t hash = fnv1_hash("somfy_rts_state_" + to_string(this->address_));
  this->store_ = global_preferences->make_preference<uint16_t>(hash ^ RESTORE_STATE_VERSION);
  this->store_.load(&this->rolling_code_);
}

void SomfyRtsRemote::dump_config() {

}

}  // namespace somfy_rts
}  // namespace esphome
