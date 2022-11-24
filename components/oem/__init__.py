# Open Energy Monitor CT Sensor
# https://openenergymonitor.org/

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import (
  CONF_ID,
  DEVICE_CLASS_CURRENT,
  STATE_CLASS_MEASUREMENT,
  UNIT_AMPERE,
)

AUTO_LOAD = ["voltage_sampler"]
MULTI_CONF = True

CONF_SAMPLE_DURATION = "sample_duration"
CONF_ZERO = "vref_zero_point" # both CT and V
CONF_CT_SENSOR = "ct_sensor"
CONF_CT_BURDEN_RESISTOR_VALUE = "ct_burden_resistor_value"
CONF_CT_TURNS = "ct_turns"
CONF_V_SENSOR = "v_sensor"
# cf: https://learn.openenergymonitor.org/electricity-monitoring/voltage-sensing/measuring-voltage-with-an-acac-power-adapter
CONF_V_TRANSO_SEC = "v_transfo_sec"
CONF_V_R1 = "v_r1"
CONF_V_R2 = "v_r2"

oem_ns = cg.esphome_ns.namespace("oem")
OemComputer = oem_ns.class_("OemComputer", cg.PollingComponent)

CONFIG_SCHEMA = (
  cv.Schema({
    cv.GenerateID(): cv.declare_id(OemComputer),
    cv.Optional(CONF_SAMPLE_DURATION, default="200ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_ZERO, default="1.65"): cv.float_range(min=0, min_included=False),
    cv.Required(CONF_CT_SENSOR): cv.use_id(voltage_sampler.VoltageSampler),
    cv.Required(CONF_CT_BURDEN_RESISTOR_VALUE): cv.positive_not_null_int,
    cv.Required(CONF_CT_TURNS): cv.positive_not_null_int,
    cv.Required(CONF_V_SENSOR): cv.use_id(voltage_sampler.VoltageSampler),
    cv.Required(CONF_V_TRANSO_SEC): cv.float_range(min=0, min_included=False),
    cv.Required(CONF_V_R1): cv.positive_not_null_int,
    cv.Required(CONF_V_R2): cv.positive_not_null_int,
  })
  .extend(cv.polling_component_schema("60s"))
)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)

  cg.add(var.set_id(str(config[CONF_ID]))) # used for logs
  cg.add(var.set_sample_duration(config[CONF_SAMPLE_DURATION]))
  cg.add(var.set_zero(config[CONF_ZERO]))

  ct_sensor = await cg.get_variable(config[CONF_CT_SENSOR])
  cg.add(var.set_ct_sensor(ct_sensor))
  cg.add(var.set_ct_burden_resistor_value(config[CONF_CT_BURDEN_RESISTOR_VALUE]))
  cg.add(var.set_ct_turns(config[CONF_CT_TURNS]))

  v_sensor = await cg.get_variable(config[CONF_V_SENSOR])
  cg.add(var.set_v_sensor(v_sensor))
  cg.add(var.set_v_transfo_sec(config[CONF_V_TRANSO_SEC]))
  cg.add(var.set_v_r1(config[CONF_V_R1]))
  cg.add(var.set_v_r2(config[CONF_V_R2]))
