import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text, light
from esphome.const import CONF_ID
from . import CONF_LIGHT_ID, fairy_lights_ns


FairyLightsEngine = fairy_lights_ns.class_("FairyLightsEngine", cg.Component, text.Text)

CONFIG_SCHEMA = text.TEXT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(FairyLightsEngine),
        cv.GenerateID(CONF_LIGHT_ID): cv.use_id(light.AddressableLightState),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text.register_text(var, config)

    light = await cg.get_variable(config[CONF_LIGHT_ID])
    cg.add(var.set_light(var, light))
