#include "controller_factory.h"

#ifdef USE_MYLIFE

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "controller.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_FAN
#include "esphome/components/fan/fan_state.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_state.h"
#endif
#ifdef USE_COVER
#include "esphome/components/cover/cover.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

#ifdef USE_BINARY_SENSOR
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, binary_sensor::BinarySensor *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'BinarySensor'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_SENSOR
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, sensor::Sensor *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Sensor'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_SWITCH
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, switch_::Switch *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Switch'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_TEXT_SENSOR
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, text_sensor::TextSensor *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'TextSensor'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_FAN
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, fan::FanState *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Fan'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_CLIMATE
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, climate::Climate *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Climate'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_LIGHT
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, light::LightState *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Light'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_COVER
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, cover::Cover *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Cover'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_NUMBER
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, number::Number *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Number'", component->get_name().c_str());
  return nullptr;
}
#endif

#ifdef USE_SELECT
static std::unique_ptr<MylifeController> create_controller(MylifeClientComponent *client, select::Select *component) {
  ESP_LOGI(TAG, "Skipping unsupported component '%s' of type 'Select'", component->get_name().c_str());
  return nullptr;
}
#endif

template <typename Components>
static void process_list(std::vector<std::unique_ptr<MylifeController>> &list, MylifeClientComponent *client, const Components &components) {
  for (auto *component : components) {
    if (component->is_internal()) {
      continue;
    }

    auto controller = create_controller(client, component);
    if (controller) {
      list.emplace_back(std::move(controller));
    }
  }
}

std::vector<std::unique_ptr<MylifeController>> MylifeControllerFactory::build(MylifeClientComponent *client) {
  std::vector<std::unique_ptr<MylifeController>> list;

#ifdef USE_BINARY_SENSOR
  process_list(list, client, App.get_binary_sensors());
#endif

#ifdef USE_SENSOR
  process_list(list, client, App.get_sensors());
#endif

#ifdef USE_SWITCH
  process_list(list, client, App.get_switches());
#endif

#ifdef USE_TEXT_SENSOR
  process_list(list, client, App.get_text_sensors());
#endif

#ifdef USE_FAN
  process_list(list, client, App.get_fans());
#endif

#ifdef USE_CLIMATE
  process_list(list, client, App.get_climates());
#endif

#ifdef USE_LIGHT
  process_list(list, client, App.get_lights());
#endif

#ifdef USE_COVER
  process_list(list, client, App.get_covers());
#endif

#ifdef USE_NUMBER
  process_list(list, client, App.get_numbers());
#endif

#ifdef USE_SELECT
  process_list(list, client, App.get_selects());
#endif

  return list;
}


}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE