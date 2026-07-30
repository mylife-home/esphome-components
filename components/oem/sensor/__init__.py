import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import (
  CONF_SENSOR,
  CONF_DEVICE_CLASS,
  CONF_UNIT_OF_MEASUREMENT,
  CONF_ACCURACY_DECIMALS,
  DEVICE_CLASS_CURRENT,
  STATE_CLASS_MEASUREMENT,
  UNIT_AMPERE,
)

from .. import oem_ns, OemComputer

AUTO_LOAD = ["voltage_sampler"]

CONF_COMPUTER = "computer"
CONF_TYPE = "type"


SensorType = oem_ns.enum("SensorType")
TYPES = {
  "current": SensorType.CURRENT,
  "voltage": SensorType.VOLTAGE,
  "apparent_power": SensorType.APPARENT_POWER,
  "real_power": SensorType.REAL_POWER,
}

SENSOR_TYPE_DEFAULTS = {
  "current": {
    CONF_DEVICE_CLASS: "current",
    CONF_UNIT_OF_MEASUREMENT: "A",
    CONF_ACCURACY_DECIMALS: 2,
  },
  "voltage": {
    CONF_DEVICE_CLASS: "voltage",
    CONF_UNIT_OF_MEASUREMENT: "V",
    CONF_ACCURACY_DECIMALS: 0,
  },
  "apparent_power": {
    CONF_DEVICE_CLASS: "apparent_power",
    CONF_UNIT_OF_MEASUREMENT: "VA",
    CONF_ACCURACY_DECIMALS: 0,
  },
  "real_power": {
    CONF_DEVICE_CLASS: "power",
    CONF_UNIT_OF_MEASUREMENT: "W",
    CONF_ACCURACY_DECIMALS: 0,
  },
}

def _apply_sensor_type_defaults(config):
    raw_type = config.get(CONF_TYPE)
    if raw_type is None:
        return config  # let cv.Required raise its own error downstream
    defaults = SENSOR_TYPE_DEFAULTS[str(raw_type).lower()]
    for key, value in defaults.items():
        config.setdefault(key, value)  # don't clobber an explicit user override
    return config


OemSensor = oem_ns.class_("OemSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = cv.All(
  _apply_sensor_type_defaults,  # runs first, while CONF_TYPE is still the raw string
  sensor.sensor_schema(
    OemSensor,
    unit_of_measurement=cv.UNDEFINED,
    accuracy_decimals=cv.UNDEFINED,
    device_class=cv.UNDEFINED,
    state_class=STATE_CLASS_MEASUREMENT,
  )
  .extend({
    cv.Required(CONF_COMPUTER): cv.use_id(OemComputer),
    cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
  })
)

async def to_code(config):
  var = await sensor.new_sensor(config)
  await cg.register_component(var, config)

  computer = await cg.get_variable(config[CONF_COMPUTER])
  cg.add(var.set_computer(computer))
  cg.add(var.set_type(config[CONF_TYPE]))
