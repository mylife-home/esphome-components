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

FairyLightsEngine::FairyLightsEngine()
 : machine_(nullptr) {
}

FairyLightsEngine::~FairyLightsEngine() {
}

void FairyLightsEngine::set_light(light::AddressableLight *light) {
  this->api_ = std::make_shared<Api>(light->all());
}


void FairyLightsEngine::loop() {
  if (!this->machine_) {
    return;
  }

  if (!tick()) {
    ESP_LOGE(TAG, "Program error, stopping.");
    this->status_set_error("Program error");

    this->machine_ = nullptr;
  }
}

void FairyLightsEngine::control(const std::string &value) {
  this->publish_state(value);

  this->machine_ = nullptr;
  this->status_clear_error();

  auto exec = Executable::from_base64(value);
  if (!exec) {
    ESP_LOGE(TAG, "Failed to load program");
    this->status_set_error("Failed to load program");
    return;
  }

  std::stringstream ss;
  std::string line;
  ss << *exec;

  ESP_LOGD(TAG, "Program:");
  while(std::getline(ss, line, '\n')){
    ESP_LOGD(TAG, "  %s", line.c_str());
  }

  this->machine_ = Machine::load_executable(*exec, this->api_);
}

bool FairyLightsEngine::tick() {
  auto &machine = *this->machine_;
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

  return true;
}


}  // namespace fairy_lights
}  // namespace esphome
