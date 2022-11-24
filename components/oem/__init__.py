# Open Energy Monitor CT Sensor
# https://openenergymonitor.org/

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import (
  CONF_ID,
  CONF_SENSOR,
  DEVICE_CLASS_CURRENT,
  STATE_CLASS_MEASUREMENT,
  UNIT_AMPERE,
)

AUTO_LOAD = ["voltage_sampler"]
MULTI_CONF = True

CONF_SAMPLE_DURATION = "sample_duration"
CONF_ZERO = "vref_zero_point"
CONF_BURDEN_RESISTOR_VALUE = "burden_resistor_value"
CONF_CT_TURNS = "ct_turns"

oem_ns = cg.esphome_ns.namespace("oem")
OemComputer = oem_ns.class_("OemComputer", cg.PollingComponent)

CONFIG_SCHEMA = (
  cv.Schema({
    cv.GenerateID(): cv.declare_id(OemComputer),
    cv.Required(CONF_SENSOR): cv.use_id(voltage_sampler.VoltageSampler),
    cv.Optional(CONF_SAMPLE_DURATION, default="200ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_ZERO, default="1.65"): cv.float_range(min=0, min_included=False),
    cv.Required(CONF_BURDEN_RESISTOR_VALUE): cv.positive_not_null_int,
    cv.Required(CONF_CT_TURNS): cv.positive_not_null_int,
  })
  .extend(cv.polling_component_schema("60s"))
)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)

  cg.add(var.set_id(str(config[CONF_ID]))) # used for logs
  sensor = await cg.get_variable(config[CONF_SENSOR])
  cg.add(var.set_source(sensor))
  cg.add(var.set_sample_duration(config[CONF_SAMPLE_DURATION]))
  cg.add(var.set_zero(config[CONF_ZERO]))
  cg.add(var.set_burden_resistor_value(config[CONF_BURDEN_RESISTOR_VALUE]))
  cg.add(var.set_ct_turns(config[CONF_CT_TURNS]))