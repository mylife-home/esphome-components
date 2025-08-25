#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text/text.h"


namespace esphome {
namespace fairy_lights {

class Executable;

class FairyLightsText : public text::Text {
public:
  explicit FairyLightsText();
  virtual ~FairyLightsText();

  virtual void control(const std::string &value) override;

public:
  const Executable* get_executable() const {
    return this->executable_.get();
  }

private:
  std::unique_ptr<Executable> executable_;
};


}  // namespace fairy_lights
}  // namespace esphome
