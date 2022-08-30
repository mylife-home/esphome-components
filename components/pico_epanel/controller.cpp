#include "controller.h"
#include "output.h"
#include "esphome/core/log.h"

#define REG_CHECK 1
#define REG_RESET 2
#define REG_INPUTS 3
#define REG_OUTPUTS 4
#define REG_INTERNAL_TEMP 5

namespace esphome {
namespace pico_epanel {

static const char *const TAG = "pico_epanel";
static constexpr uint16_t magic = 0x4242;

void PicoEpanelController::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PicoEpanelController...");

  // check magic
  uint16_t read_magic;
  if (!this->read_u16(REG_CHECK, &read_magic)) {
    this->mark_failed();
    return;
  }

  if (read_magic != magic) {
    ESP_LOGE(TAG, "Wrong magic %hu", read_magic);
    this->mark_failed();
    return;
  }

  // reset
  if (!this->write_u16(REG_RESET, 0)) {
    this->mark_failed();
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
    this->status_clear_warning();
    return true;
  }

  ESP_LOGE(TAG, "Could not read register %hu (error=%d)", reg, result);
  this->status_set_warning();
  return false;
}

bool PicoEpanelController::write_u16(uint8_t reg, const uint16_t value) {
  auto result = this->write_register(reg, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
  if (result == i2c::ERROR_OK) {
    this->status_clear_warning();
    return true;
  }

  ESP_LOGE(TAG, "Could not write register %hu (error=%d)", reg, result);
  this->status_set_warning();
  return false;
}

std::bitset<16> PicoEpanelController::read_inputs() {
  uint16_t data;
  if (!this->read_u16(REG_INPUTS, &data)) {
    return 0;
  }

  ESP_LOGD(TAG, "Read inputs %hu", data);

  return {data};
}


// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
static constexpr const float conversion_factor = 3.3f / (1 << 12);

float PicoEpanelController::read_internal_temp() {
  uint16_t raw;

  if (!this->read_u16(REG_INTERNAL_TEMP, &raw)) {
    return 0;
  }

  float adc_voltage = raw * conversion_factor;
  // T = 27 - (ADC_Voltage - 0.706)/0.001721
  float temp = 27 - (adc_voltage - 0.706) / 0.001721;

  ESP_LOGD(TAG, "Read internal temp %f C (raw=%hu)", temp, raw);
}

inline uint8_t color_ftou(float value) {
  return static_cast<uint8_t>(value * 255);
}

void PicoEpanelController::set_output(PicoEpanelOutput *output, uint8_t index) {
  output->add_on_write_callback([&, index](float value) {
    this->defer([&, index, value]() {
      this->write_output(index, color_ftou(value));
    });
  });
}

void PicoEpanelController::write_output(uint8_t index, uint8_t value) {
  if (this->is_failed()) {
    return;
  }
  
  ESP_LOGD(TAG, "Write output @ %d -> %d", index, value);

  uint16_t data = ((uint16_t)index << 8) | value;
  this->write_u16(REG_OUTPUTS, data);
}

void PicoEpanelController::s_intr_pin_handler(PicoEpanelController *this_) {
  this_->defer([this_]() {
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
