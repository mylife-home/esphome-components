import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_INDEX
from . import PicoEpanelController, CONF_CONTROLLER_ID, pico_epanel_ns

DEPENDENCIES = ["pico_epanel"]

PicoEpanelOutput = pico_epanel_ns.class_("PicoEpanelOutput", cg.Component, output.FloatOutput)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(cv.COMPONENT_SCHEMA).extend(
    {
        cv.Required(CONF_ID): cv.declare_id(PicoEpanelOutput),
        cv.GenerateID(CONF_CONTROLLER_ID): cv.use_id(PicoEpanelController),
        cv.Required(CONF_INDEX): cv.int_range(min=0, max=15),
        cv.Optional(output.CONF_ZERO_MEANS_ZERO, default=True): cv.boolean, # override default
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)

    controller = await cg.get_variable(config[CONF_CONTROLLER_ID])
    cg.add(controller.set_output(var, config[CONF_INDEX]))
