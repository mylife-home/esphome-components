#include "controller_text.h"

#ifdef USE_MYLIFE
#ifdef USE_TEXT

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "text.simple",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "simple",
  "module": "text",
  "usage": "actuator",
  "version": "{{version}}",
  "config": {},
  "members": {
    "value": { "memberType": "state", "valueType": "text" },
    "setValue": { "memberType": "action", "valueType": "text" }
  }
}
)EOF")
};

MylifeText::MylifeText(MylifeClientComponent *client, text::Text *text)
  : MylifeController(client, text)
  , text_(text) {

    this->subscribe_action("setValue", [this](const std::string &buffer) {
      this->on_set_value(Encoding::read_string(buffer));
    });
  
    this->text_->add_on_state_callback([this](std::string) {
      this->on_text_change();
    });
}

const PluginDefinition *MylifeText::get_plugin_metadata() const {
  return &definition;
}

void MylifeText::publish_states() {
  this->publish_states_(true);
}


void MylifeText::on_set_value(const std::string &value) {
  auto call = this->text_->make_call();
  call.set_value(value);
  call.perform();
}

void MylifeText::on_text_change() {
  this->publish_states_(false);
}

void MylifeText::publish_states_(bool force) {
  const auto &value = this->text_->state;

  if (force || value != this->value_) {
    this->value_ = value;
    this->publish_state("value", Encoding::write_string(this->value_));
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_TEXT
#endif  // USE_MYLIFE
