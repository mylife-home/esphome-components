import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import i2c
from esphome.const import (
    CONF_ID,
    CONF_PIN,
    CONF_ADDRESS
)

MULTI_CONF = True

somfy_rts_ns = cg.esphome_ns.namespace("somfy_rts")

SomfyRtsRemote = somfy_rts_ns.class_("SomfyRtsRemote", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(SomfyRtsRemote),
            cv.Required(CONF_PIN): pins.internal_gpio_output_pin_schema,
            cv.Required(CONF_ADDRESS): cv.All(cv.hex_int, cv.Range(min=0, max=0xFFFFFF)),
        }
    )
)


async def to_code(config):
    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    address = config[CONF_ADDRESS]

    var = cg.new_Pvariable(config[CONF_ID], pin, address)
    await cg.register_component(var, config)
