import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_INDEX
from . import PicoEpanelController, CONF_CONTROLLER_ID

DEPENDENCIES = ["pico_epanel"]

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(CONF_CONTROLLER_ID): cv.use_id(PicoEpanelController),
        cv.Required(CONF_INDEX): cv.int_range(min=0, max=15),
    }
)

async def to_code(config):
    controller = await cg.get_variable(config[CONF_CONTROLLER_ID])
    var = await binary_sensor.new_binary_sensor(config)
    cg.add(controller.set_input(var, config[CONF_INDEX]))
