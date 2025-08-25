import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text
from esphome.const import CONF_ID, CONF_MODE

fairy_lights_ns = cg.esphome_ns.namespace("fairy_lights")

FairyLightsText = fairy_lights_ns.class_("FairyLightsText", text.Text)

CONFIG_SCHEMA = text.text_schema().extend({
    cv.GenerateID(): cv.declare_id(FairyLightsText),
    cv.Optional(CONF_MODE, default="text"): cv.enum(text.TEXT_MODES, upper=True),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await text.register_text(var, config, max_length=4096)
