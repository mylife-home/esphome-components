import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import PicoEpanelController, CONF_CONTROLLER_ID, pico_epanel_ns

DEPENDENCIES = ["pico_epanel"]

InternalTempSensor = pico_epanel_ns.class_(
    "InternalTempSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = sensor.sensor_schema(
        InternalTempSensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend(
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
