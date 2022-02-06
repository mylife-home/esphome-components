#include "controller.h"
#include "esphome/core/log.h"

#define REG_CHECK 1
#define REG_RESET 2
#define REG_INPUTS 3
#define REG_OUTPUTS 4


namespace esphome {
namespace pico_epanel {

static const char *const TAG = "pico_epanel";
static constexpr uint16_t magic = 0x4242;

void PicoEpanelController::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PicoEpanelController...");

  // check magic
  uint16_t read_magic;
  if (!this->read_u16(REG_CHECK, &read_magic)) {
    return;
  }

  if (read_magic != magic) {
    ESP_LOGE(TAG, "Wrong magic %hu", read_magic);
    this->mark_failed();
    return;
  }

  // reset
  if (!this->write_u16(REG_RESET, 0)) {
    return;
  }

  // initial inputs state
  this->refresh_inputs();

  // setup interrupt pin
  this->intr_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->intr_pin_->setup();
  this->intr_pin_->attach_interrupt(&PicoEpanelController::s_intr_pin_handler, this, gpio::INTERRUPT_FALLING_EDGE);
}

void PicoEpanelController::dump_config() {
  ESP_LOGCONFIG(TAG, "PicoEpanelController:");
  if (this->is_failed()) {
    ESP_LOGCONFIG(TAG, "  failed!");
  }
}

bool PicoEpanelController::read_u16(uint8_t reg, uint16_t *value) {
  auto result = this->read_register(reg, reinterpret_cast<uint8_t *>(value), sizeof(*value));
  if (result == i2c::ERROR_OK) {
    return true;
  }

  ESP_LOGE(TAG, "Could not read register %hu (error=%d)", reg, result);
  this->mark_failed();
  return false;
}

bool PicoEpanelController::write_u16(uint8_t reg, const uint16_t value) {
  auto result = this->write_register(reg, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
  if (result == i2c::ERROR_OK) {
    return true;
  }

  ESP_LOGE(TAG, "Could not write register %hu (error=%d)", reg, result);
  this->mark_failed();
  return false;
}

std::bitset<16> PicoEpanelController::read_inputs() {
  uint16_t data;
  if (!this->read_u16(REG_INPUTS, &data)) {
    return 0;
  }

  return {data};
}

void PicoEpanelController::write_output(uint8_t index, uint8_t value) {
  uint16_t data = ((uint16_t)index << 8) | value;
  this->write_u16(REG_OUTPUTS, data);
}

void PicoEpanelController::s_intr_pin_handler(PicoEpanelController *this_) {
  this_->defer([&]() {
    this_->refresh_inputs();
  });
}

void PicoEpanelController::refresh_inputs() {
  auto values = read_inputs();

  for (uint8_t index = 0; index < 16; ++index) {
    auto sensor = inputs_[index];
    if (sensor) {
      sensor->publish_state(values.test(index));
    }
  }
}

}  // namespace pico_epanel
}  // namespace esphome
