#include <sstream>
#include <string>

#include "text.h"
#include "esphome/core/log.h"

#include "executable.h"

static const char *const TAG = "fairy_lights";

namespace esphome {
namespace fairy_lights {

FairyLightsText::FairyLightsText() {
}

FairyLightsText::~FairyLightsText() {
}

void FairyLightsText::control(const std::string &value) {
  if (value.empty()) {
    this->executable_ = nullptr;
    this->publish_state(value);
    return;
  }

  auto exec = Executable::from_base64(value);
  if (!exec) {
    ESP_LOGE(TAG, "Failed to load program");
    return;
  }

  this->executable_ = std::make_unique<Executable>(std::move(*exec));

  std::stringstream ss;
  std::string line;
  ss << *this->executable_;

  ESP_LOGD(TAG, "Program:");
  while (std::getline(ss, line, '\n')) {
    ESP_LOGD(TAG, "  %s", line.c_str());
  }

  this->publish_state(value);
}

}  // namespace fairy_lights
}  // namespace esphome
