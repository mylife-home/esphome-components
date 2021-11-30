#include "controller_switch.h"

#ifdef USE_MYLIFE
#ifdef USE_SWITCH

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"
#include "color_utils.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "switch.binary",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "binary",
  "module": "switch",
  "usage": "actuator",
  "version": "{{version}}",
  "config": {},
  "members": {
    "value": { "memberType": "state", "valueType": "bool" },
    "setValue": { "memberType": "action", "valueType": "bool" }
  }
}
)EOF")
};

MylifeSwitch::MylifeSwitch(MylifeClientComponent *client, switch_::Switch *target)
  : MylifeController(client, target)
  , switch_(target) {

  subscribe_action("setValue", [this](const std::string &buffer) {
    this->on_set_value(Encoding::read_bool(buffer));
  });

  this->switch_->add_on_state_callback([this](bool) {
    this->on_switch_change();
  });
}

const PluginDefinition *MylifeSwitch::get_plugin_metadata() const {
  return &definition;
}

void MylifeSwitch::publish_states() {
  this->publish_states_(true);
}

void MylifeSwitch::on_set_value(bool value) {
  if (value) {
    this->switch_->turn_on();
  } else {
    this->switch_->turn_off();
  }
}

void MylifeSwitch::on_switch_change() {
  this->publish_states_(false);
}

void MylifeSwitch::publish_states_(bool force) {
  auto value = this->switch_->state;

  if (force || value != value_) {
    value_ = value;
    this->publish_state("value", Encoding::write_bool(value_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_SWITCH
#endif  // USE_MYLIFE
