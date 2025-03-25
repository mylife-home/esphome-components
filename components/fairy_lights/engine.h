#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text/text.h"
#include "esphome/components/light/addressable_light.h"


namespace esphome {
namespace fairy_lights {

class Machine;
class Api;

class FairyLightsEngine : public text::Text, public Component {
public:
  explicit FairyLightsEngine();
  virtual ~FairyLightsEngine();

  void set_light(light::AddressableLight *light);

  void loop() override;

protected:
  void control(const std::string &value) override;

private:
  bool tick();

  std::unique_ptr<Machine> machine_;
  std::shared_ptr<Api> api_;
};


}  // namespace fairy_lights
}  // namespace esphome
