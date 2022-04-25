#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MQTT_STUB

// Override this only to keep its logging capabilities

namespace esphome {
namespace mqtt {

class MQTTComponentStub {
 public:
  MQTTComponentStub(void *inner) {
  }
};

}  // namespace mqtt
}  // namespace esphome

#endif  // USE_MQTT_STUB
