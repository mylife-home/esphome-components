#include <unordered_map>
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

// Only keep one interrupt pin instance, shared across controller if they use same pin number.
// Else multiple init of same pin overwrite each other
class SharedInterruptPin {
public:
  SharedInterruptPin(InternalGPIOPin *pin)
   : pin_(pin) {
  }

  void setup() {
    this->pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    this->pin_->setup();
    this->pin_->attach_interrupt(&SharedInterruptPin::s_pin_handler, this, gpio::INTERRUPT_FALLING_EDGE);
  }

  void register_callback(std::function<void()> &&callback) {
    this->callback_.add(std::move(callback));
  }

 InternalGPIOPin *pin() const {
    return this->pin_;
  }

  // here we are cheating:
  // we will run every callback from the first component that created the shared pin
  void loop() {
    bool pending_callback = true;
    this->pending_callback_.compare_exchange_strong(pending_callback, false);

    if (pending_callback) {
      this->callback_.call();
    }
  }

private:
  IRAM_ATTR HOT static void s_pin_handler(SharedInterruptPin *this_) {
    this_->pending_callback_ = true;
  }

  CallbackManager<void()> callback_{};
  InternalGPIOPin *pin_{nullptr};
  std::atomic<bool> pending_callback_{false};
};

static std::unordered_map<uint8_t, SharedInterruptPin> interrupt_pins;

PicoEpanelController::PicoEpanelController(std::string id)
 : id_(std::move(id)) {
}

void PicoEpanelController::setup() {
  ESP_LOGCONFIG(TAG, "[%s] Setting up PicoEpanelController...", id_.c_str());

  // check magic
  uint16_t read_magic;
  if (!this->read_u16(REG_CHECK, &read_magic)) {
    this->mark_failed();
    return;
  }

  if (read_magic != magic) {
    ESP_LOGE(TAG, "[%s] Wrong magic %hu", id_.c_str(), read_magic);
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
  auto pin = this->intr_pin_->get_pin();
  std::unordered_map<uint8_t, SharedInterruptPin>::iterator iter;
  bool inserted;
  std::tie(iter, inserted) = interrupt_pins.emplace(pin, this->intr_pin_);
  auto &shared_intr_pin = iter->second;

  if (inserted) {
    shared_intr_pin.setup();
    this->owned_intr_pin_ = &shared_intr_pin;
  }

  this->intr_pin_ = shared_intr_pin.pin();

  shared_intr_pin.register_callback([this]() {
    this->refresh_inputs();
  });
}

void PicoEpanelController::loop() {
  if (this->owned_intr_pin_) {
    this->owned_intr_pin_->loop();
  }
}

void PicoEpanelController::dump_config() {
  ESP_LOGCONFIG(TAG, "[%s] PicoEpanelController:", id_.c_str());
  ESP_LOGCONFIG(TAG, "  Interrupt pin: %s", this->intr_pin_->dump_summary().c_str());
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

  ESP_LOGE(TAG, "[%s] Could not read register %hu (error=%d)", id_.c_str(), reg, result);
  this->status_set_warning();
  return false;
}

bool PicoEpanelController::write_u16(uint8_t reg, const uint16_t value) {
  auto result = this->write_register(reg, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
  if (result == i2c::ERROR_OK) {
    this->status_clear_warning();
    return true;
  }

  ESP_LOGE(TAG, "[%s] Could not write register %hu (error=%d)", id_.c_str(), reg, result);
  this->status_set_warning();
  return false;
}

std::bitset<16> PicoEpanelController::read_inputs() {
  uint16_t data;
  if (!this->read_u16(REG_INPUTS, &data)) {
    return 0;
  }

  ESP_LOGD(TAG, "[%s] Read inputs %hu", id_.c_str(), data);

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

  ESP_LOGD(TAG, "[%s] Read internal temp %f C (raw=%hu)", id_.c_str(), temp, raw);

  return temp;
}

inline uint8_t color_ftou(float value) {
  return static_cast<uint8_t>(value * 255);
}

void PicoEpanelController::set_output(PicoEpanelOutput *output, uint8_t index) {
  output->add_on_write_callback([this, index](float value) {
    this->write_output(index, color_ftou(value));
  });
}

void PicoEpanelController::write_output(uint8_t index, uint8_t value) {
  if (this->is_failed()) {
    return;
  }
  
  ESP_LOGD(TAG, "[%s] Write output @ %d -> %d", id_.c_str(), index, value);

  uint16_t data = ((uint16_t)index << 8) | value;
  this->write_u16(REG_OUTPUTS, data);
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
