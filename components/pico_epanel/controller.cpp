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
  this->write_u16(REG_RESET, 0);
}

void PicoEpanelController::loop() {
  auto new_inputs = read_inputs();
  if (new_inputs == this->inputs_) {
    return;
  }

  // TODO
  ESP_LOGI(TAG, "Inputs change %hu -> %hu", this->inputs_, new_inputs);

  this->inputs_ = new_inputs;
}

void PicoEpanelController::dump_config() {
  ESP_LOGCONFIG(TAG, "PicoEpanelController:");
  if (this->is_failed()) {
    ESP_LOGCONFIG(TAG, "  failed!");
  }
}

bool PicoEpanelController::read_u16(uint8_t reg, uint16_t *value) {
  if (this->read_register(reg, reinterpret_cast<uint8_t *>(value), sizeof(*value)) == i2c::ERROR_OK) {
    return true;
  }

  ESP_LOGE(TAG, "Could not read register %hu", reg);
  this->mark_failed();
  return false;
}

bool PicoEpanelController::write_u16(uint8_t reg, const uint16_t value) {
  if (this->write_register(reg, reinterpret_cast<const uint8_t *>(&value), sizeof(value)) == i2c::ERROR_OK) {
    return true;
  }

  ESP_LOGE(TAG, "Could not write register %hu", reg);
  this->mark_failed();
  return false;
}

uint16_t PicoEpanelController::read_inputs() {
  uint16_t data;
  if (!this->read_u16(REG_INPUTS, &data)) {
    return 0;
  }

  return data;
}

void PicoEpanelController::write_output(uint8_t index, uint8_t value) {
  uint16_t data = ((uint16_t)index << 8) | value;
  this->write_u16(REG_OUTPUTS, data);
}

}  // namespace pico_epanel
}  // namespace esphome
