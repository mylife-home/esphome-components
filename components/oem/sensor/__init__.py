import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import (
  CONF_SENSOR,
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

OemSensor = oem_ns.class_("OemSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = (
  sensor.sensor_schema(
    OemSensor,
    # will be set internally depending of the type of sensor
    unit_of_measurement=cv.UNDEFINED,
    accuracy_decimals=cv.UNDEFINED,
    device_class=cv.UNDEFINED,
    state_class=cv.UNDEFINED,
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
