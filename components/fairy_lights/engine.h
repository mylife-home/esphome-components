#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text/text.h"
#include "esphome/components/light/addressable_light_effect.h"
#include "text.h"


namespace esphome {
namespace fairy_lights {

class Machine;
class Api;
class Executable;

class FairyLightsEngine : public light::AddressableLightEffect {
public:
  explicit FairyLightsEngine(const std::string &name);
  virtual ~FairyLightsEngine();

  void set_text(FairyLightsText *text);

  void start() override;
  void stop() override;
  void apply(light::AddressableLight &it, const Color &current_color) override;

private:
  void reset();
  bool tick();

  const Executable *exec_;
  bool error_{false};

  light::AddressableLight *light_{nullptr};
  std::unique_ptr<Machine> machine_;
  std::shared_ptr<Api> api_;
};


}  // namespace fairy_lights
}  // namespace esphome
