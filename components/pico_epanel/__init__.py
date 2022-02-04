import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

pico_epanel_ns = cg.esphome_ns.namespace("pico_epanel")

PicoEpanelController = pico_epanel_ns.class_("PicoEpanelController", cg.Component, i2c.I2CDevice)

CONF_INTERRUPT_PIN = "interrupt_pin"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(PicoEpanelController),
            cv.Required(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        }
    )
    .extend(i2c.i2c_device_schema(0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await i2c.register_i2c_device(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
    cg.add(var.set_interrupt_pin(pin))
