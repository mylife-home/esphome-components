import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import (
  CONF_SENSOR,
  DEVICE_CLASS_CURRENT,
  STATE_CLASS_MEASUREMENT,
  UNIT_AMPERE,
)

from .. import oem_clamp_ns, OemClamp

AUTO_LOAD = ["voltage_sampler"]

CONF_OEM_CLAMP = "oem_clamp"
CONF_TYPE = "type"


SensorType = oem_clamp_ns.enum("SensorType")
TYPES = {
  "current": SensorType.CURRENT,
  "voltage": SensorType.VOLTAGE,
  "apparent_power": SensorType.APPARENT_POWER,
  "real_power": SensorType.REAL_POWER,
}

OemClampSensor = oem_clamp_ns.class_("OemClampSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = (
  sensor.sensor_schema(
    OemClampSensor,
    # will be set inteenrally depending of the type of sensor
    unit_of_measurement=sensor._UNDEF,
    accuracy_decimals=sensor._UNDEF,
    device_class=sensor._UNDEF,
    state_class=sensor._UNDEF,
  )
  .extend({
    cv.Required(CONF_OEM_CLAMP): cv.use_id(OemClamp),
    cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
  })
)

async def to_code(config):
  var = await sensor.new_sensor(config)
  await cg.register_component(var, config)

  oem_clamp = await cg.get_variable(config[CONF_OEM_CLAMP])
  cg.add(var.set_oem_clamp(oem_clamp))
  cg.add(var.set_type(config[CONF_TYPE]))
