import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.effects import register_addressable_effect
from esphome.components.light.types import AddressableLightEffect
from esphome.components import text
from esphome.const import CONF_NAME

CONF_PROGRAM_ID = "program"

fairy_lights_ns = cg.esphome_ns.namespace("fairy_lights")

FairyLightsEngine = fairy_lights_ns.class_("FairyLightsEngine", AddressableLightEffect)

CONFIG_SCHEMA = cv.Schema({})


@register_addressable_effect(
    "fairy_lights",
    FairyLightsEngine,
    "Fairy Lights",
    {cv.Required(CONF_PROGRAM_ID): cv.use_id(text.Text)},
)
async def fairy_lights_light_effect_to_code(config, effect_id):
    effect = cg.new_Pvariable(effect_id, config[CONF_NAME])
    program = await cg.get_variable(config[CONF_PROGRAM_ID])

    cg.add(effect.set_text(program))

    return effect
