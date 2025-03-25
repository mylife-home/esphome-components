#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text/text.h"
// #include "esphome/components/light/addressable_light.h"


namespace esphome {
namespace fairy_lights {

class Machine;

class FairyLightsEngine : public text::Text, public Component {
public:
  explicit FairyLightsEngine();
  virtual ~FairyLightsEngine();

  // void set_light(light::AddressableLight *light) { 
  //   this->light_ = light;
  // }

  void setup() override;
  void loop() override;
  void dump_config() override;

protected:
  void control(const std::string &value) override;

private:
  bool tick();

  // light::AddressableLight *light_{nullptr};
  std::unique_ptr<Machine> machine_;
};


}  // namespace fairy_lights
}  // namespace esphome
