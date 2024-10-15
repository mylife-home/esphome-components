#include "controller_climate.h"

#ifdef USE_MYLIFE
#ifdef USE_CLIMATE

#include "controller.h"
#include "metadata.h"
#include "version.h"
#include "encoding.h"
#include "color_utils.h"
#include "esphome/core/application.h"

namespace esphome {
namespace mylife {

static PluginDefinition definition {
  .id = "climate.simple",
  .metadata = PluginDefinition::make_meta(R"EOF(
{
  "name": "simple",
  "module": "climate",
  "usage": "actuator",
  "version": "{{version}}",
  "config": {},
  "members": {
    "targetTemperature": { "memberType": "state", "valueType": "range[17;30]" },
    "mode": { "memberType": "state", "valueType": "enum{off,heat-cool,cool,heat,dry,fan-only}" },
    "fanMode": { "memberType": "state", "valueType": "enum{auto,low,medium,high,quiet}" },
    "setTargetTemperature": { "memberType": "action", "valueType": "range[17;30]" },
    "setMode": { "memberType": "action", "valueType": "enum{off,heat-cool,cool,heat,dry,fan-only}" },
    "setFanMode": { "memberType": "action", "valueType": "enum{auto,low,medium,high,quiet}" }
  }
}
)EOF")
};

static climate::ClimateMode enum_to_mode(const std::string &value);
static const char *mode_to_enum(climate::ClimateMode value);
static climate::ClimateFanMode enum_to_fan_mode(const std::string &value);
static const char *fan_mode_to_enum(climate::ClimateFanMode value);

ClimateCallDebounce::ClimateCallDebounce(climate::Climate *target)
  : climate_(target) {
}

void ClimateCallDebounce::debounced_call(std::function<void(climate::ClimateCall &)> &&f) {
  if (this->call_) {
    this->cancel_timeout("debounce");
  } else {
    this->call_.emplace(this->climate_->make_call());
  }

  f(*this->call_);

  this->set_timeout("debounce", 300, [this]() {
    this->call_->perform();
    this->call_.reset();
  });
}

MylifeClimate::MylifeClimate(MylifeClientComponent *client, climate::Climate *target)
  : MylifeController(client, target)
  , climate_(target)
  , call_(std::make_unique<ClimateCallDebounce>(target)) {
  
  // Not very clean :/
  App.register_component(this->call_.get());

  subscribe_action("setTargetTemperature", [this](const std::string &buffer) {
    this->on_set_target_temperature(static_cast<float>(Encoding::read_uint8(buffer)));
  });

  subscribe_action("setMode", [this](const std::string &buffer) {
    this->on_set_mode(enum_to_mode(Encoding::read_string(buffer)));
  });

  subscribe_action("setFanMode", [this](const std::string &buffer) {
    this->on_set_fan_mode(enum_to_fan_mode(Encoding::read_string(buffer)));
  });

  climate_->add_on_state_callback([this](climate::Climate &) {
    this->on_climate_change();
  });
}

const PluginDefinition *MylifeClimate::get_plugin_metadata() const {
  return &definition;
}

void MylifeClimate::publish_states() {
  this->publish_states_(true);
}

void MylifeClimate::on_set_target_temperature(float value) {
  this->call_->debounced_call([value](climate::ClimateCall &call) {
    call.set_target_temperature(value);
  });
}

void MylifeClimate::on_set_mode(climate::ClimateMode value) {
  this->call_->debounced_call([value](climate::ClimateCall &call) {
    call.set_mode(value);
  });
}

void MylifeClimate::on_set_fan_mode(climate::ClimateFanMode value) {
  this->call_->debounced_call([value](climate::ClimateCall &call) {
    call.set_fan_mode(value);
  });
}

void MylifeClimate::on_climate_change() {
  this->publish_states_(false);
}

void MylifeClimate::publish_states_(bool force) {
  if (force || this->target_temperature_ != this->climate_->target_temperature) {
    this->target_temperature_ = this->climate_->target_temperature;
    this->publish_state("targetTemperature", Encoding::write_uint8(static_cast<uint8_t>(this->target_temperature_)));
  }

  if (force || this->mode_ != this->climate_->mode) {
    this->mode_ = this->climate_->mode;
    this->publish_state("mode", Encoding::write_string(mode_to_enum(this->mode_)));
  }

  if (force || this->fan_mode_ != this->climate_->fan_mode) {
    if (this->climate_->fan_mode.has_value()) {
      this->fan_mode_ = *(this->climate_->fan_mode);
    }
    this->publish_state("fanMode", Encoding::write_string(fan_mode_to_enum(this->fan_mode_)));
  }
}

climate::ClimateMode enum_to_mode(const std::string &value) {
  if (value == "off") {
    return climate::CLIMATE_MODE_OFF;
  } else if (value == "heat-cool") {
    return climate::CLIMATE_MODE_HEAT_COOL;
  } else if (value == "cool") {
    return climate::CLIMATE_MODE_COOL;
  } else if (value == "heat") {
    return climate::CLIMATE_MODE_HEAT;
  } else if (value == "dry") {
    return climate::CLIMATE_MODE_DRY;
  } else if (value == "fan-only") {
    return climate::CLIMATE_MODE_FAN_ONLY;
  }

  return climate::CLIMATE_MODE_OFF; // ?!?
}

const char *mode_to_enum(climate::ClimateMode value) {
  switch(value) {
  case climate::CLIMATE_MODE_OFF:
    return "off";
  case climate::CLIMATE_MODE_HEAT_COOL:
    return "heat-cool";
  case climate::CLIMATE_MODE_COOL:
    return "cool";
  case climate::CLIMATE_MODE_HEAT:
    return "heat";
  case climate::CLIMATE_MODE_DRY:
    return "dry";
  case climate::CLIMATE_MODE_FAN_ONLY:
    return "fan-only";
  default:
    return "???";
  }
}

climate::ClimateFanMode enum_to_fan_mode(const std::string &value) {
  if (value == "auto") {
    return climate::CLIMATE_FAN_AUTO;
  } else if (value == "low") {
    return climate::CLIMATE_FAN_LOW;
  } else if (value == "medium") {
    return climate::CLIMATE_FAN_MEDIUM;
  } else if (value == "high") {
    return climate::CLIMATE_FAN_HIGH;
  } else if (value == "quiet") {
    return climate::CLIMATE_FAN_QUIET;
  }

  return climate::CLIMATE_FAN_AUTO; // ?!?
}

static const char *fan_mode_to_enum(climate::ClimateFanMode value) {
  switch(value) {
  case climate::CLIMATE_FAN_AUTO:
    return "auto";
  case climate::CLIMATE_FAN_LOW:
    return "low";
  case climate::CLIMATE_FAN_MEDIUM:
    return "medium";
  case climate::CLIMATE_FAN_HIGH:
    return "high";
  case climate::CLIMATE_FAN_QUIET:
    return "quiet";
  default:
    return "???";
  }
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_CLIMATE
#endif  // USE_MYLIFE
