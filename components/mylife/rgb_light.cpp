#include "rgb_light.h"

#ifdef USE_MYLIFE
#ifdef USE_LIGHT

#include "controller.h"
#include "metadata.h"
#include "version.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "light.rgb",
  .metadata = "{\"name\":\"rgb\",\"module\":\"light\",\"usage\":\"actuator\",\"version\":\"" MYLIFE_VERSION "\",\"config\":{},\"members\":{\"active\":{\"memberType\":\"state\",\"valueType\":\"bool\"},\"color\":{\"memberType\":\"state\",\"valueType\":\"range[0;16777215]\"},\"setActive\":{\"memberType\":\"action\",\"valueType\":\"bool\"},\"setColor\":{\"memberType\":\"action\",\"valueType\":\"range[0;16777215]\"}}}"
};

MylifeRgbLight::MylifeRgbLight(MylifeClientComponent *client, light::LightState *target)
  : MylifeController(client, target) {
}

const PluginDefinition *MylifeRgbLight::get_plugin_metadata() const {
  return &definition;
}

void MylifeRgbLight::publish_status() {
  // TODO
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_LIGHT
#endif  // USE_MYLIFE
