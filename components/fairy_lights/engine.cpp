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

static const std::string test_program = "AL66AKmJ/ThkAAAABAAAAAABAAACAgAAAGQAAAIDAAAAAAAAAQIAAA4AAAACAAAAAQAAAAECAAANAAAAAgAAAAEDAAABAAAABwAAAAwCAAALAwAACwgAAAsBAAABAAAAAAAAAAAAAAAA/wAAGAAAAAvw//8AAAAAAgEAAAABAAAKAAAADAIAAAsDAAALFQAACwEAAAAUAAAZAAAAAQEAAAAAAAAAAAAAAP8AABgAAAABAQAAAAEAAA0AAAAUAAAAEgAAAAIBAAABAQAAAP8AAAD/AAAA/wAAGAAAAAvo//8";


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
  auto exec = Executable::from_base64(test_program);
  if (!exec) {
    ESP_LOGE(TAG, "Failed to load program");
    return;
  }

  this->machine_ = Machine::load_executable(*exec, std::make_shared<Api>());
}

FairyLightsEngine::~FairyLightsEngine() {
}

void FairyLightsEngine::setup() {
  auto program = Executable::from_base64(test_program);
  if (!program) {
    ESP_LOGE(TAG, "Failed to load program");
    return;
  }

  std::stringstream ss;
  std::string line;
  ss << *program;

  ESP_LOGD(TAG, "Program:");
  while(std::getline(ss, line, '\n')){
    ESP_LOGD(TAG, "  %s", line.c_str());
  }
}

void FairyLightsEngine::loop() {
  if (!this->machine_) {
    return;
  }

  if (!tick()) {
    // set error state
    ESP_LOGE(TAG, "Program error, stopping.");
    this->machine_ = nullptr;
  }
}

void FairyLightsEngine::dump_config() {
}

void FairyLightsEngine::control(const std::string &value) {

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
