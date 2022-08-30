import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import PicoEpanelController, CONF_CONTROLLER_ID

DEPENDENCIES = ["pico_epanel"]

InternalTempSensor = pico_epanel_ns.class_(
    "InternalTempSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(InternalTempSensor),
        cv.GenerateID(CONF_CONTROLLER_ID): cv.use_id(PicoEpanelController)
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
    )
    await cg.register_parented(var, config[CONF_CONTROLLER_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)

    controller = await cg.get_variable(config[CONF_CONTROLLER_ID])
    cg.add(controller.set_internal_temp_sensor(var))

