#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text/text.h"
// #include "esphome/components/light/addressable_light.h"


namespace esphome {
namespace fairy_lights {

class FairyLightsEngine : public text::Text, public Component {
public:
  explicit FairyLightsEngine();

  // void set_light(light::AddressableLight *light) { 
  //   this->light_ = light;
  // }

  void setup() override;
  void loop() override;
  void dump_config() override;

protected:
  void control(const std::string &value) override;

private:
  // light::AddressableLight *light_{nullptr};
};


}  // namespace fairy_lights
}  // namespace esphome
