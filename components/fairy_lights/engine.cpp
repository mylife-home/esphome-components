#include <sstream>
#include <string>

#include "engine.h"
#include "esphome/core/log.h"

#include "executable.h"
#include "machine.h"
#include "instructions.h"


namespace esphome {
namespace fairy_lights {

static const char *const TAG = "fairy_lights";

class LoopGuard {
public:
  bool next() {
    constexpr std::size_t LIMIT = 10000;

    this->count += 1;
    if (this->count > LIMIT) {
      ESP_LOGE(TAG, "Infinite loop detected");
      return false;
    }

    return true;
  }

private:
  std::size_t count{0};
};

FairyLightsEngine::FairyLightsEngine(const std::string &name)
 : light::AddressableLightEffect(name) {
}

FairyLightsEngine::~FairyLightsEngine() {
}

void FairyLightsEngine::set_text(FairyLightsText *text) {
  text->add_on_state_callback([this, text](std::string) {
    this->exec_ = text->get_executable();
    this->error_ = false;
    this->machine_ = nullptr;
  });
}

void FairyLightsEngine::start() {
}

void FairyLightsEngine::stop() {
  this->reset();
}

void FairyLightsEngine::apply(light::AddressableLight &it, const Color &current_color) {
  if (!this->api_) {
    this->light_ = &it;
    this->api_ = std::make_shared<Api>(it.all());
  }
  
  // Check if we should start the machine
  if (!this->machine_ && this->exec_ && !this->error_) {
    this->machine_ = Machine::load_executable(*this->exec_, this->api_);
    ESP_LOGD(TAG, "Start VM");
  }

  if (!this->machine_) {
    return;
  }

  if (!tick()) {
    this->reset();
    ESP_LOGE(TAG, "Program error, stopping.");
    this->error_ = true;
  }
}

void FairyLightsEngine::reset() {
  this->machine_ = nullptr;
  this->error_ = false;

  if (this->light_) {
    for (auto led : this->light_->all()) {
      led.set(Color::BLACK);
    }
  }

  ESP_LOGD(TAG, "Reset");
}

bool FairyLightsEngine::tick() {
  auto &machine = *this->machine_;
  
  if (machine.sleeping()) {
    return true;
  }
  
  LoopGuard guard;
  while (true) {
    if (machine.sleeping()) {
      break;
    }

    if (!guard.next()) {
      return false;
    }

    auto opcode = machine.fetch_instruction();
    if (!opcode) {
      return false;
    }

    if (!instructions::execute(machine, *opcode)) {
      std::ostringstream oss;
      oss << *opcode;
      auto str = oss.str();
      ESP_LOGE(TAG, "Error executing instruction %s", str.c_str());
  
      return false;
    }
  }

  // Hopefully light states have changed
  this->light_->schedule_show();

  return true;
}


}  // namespace fairy_lights
}  // namespace esphome
