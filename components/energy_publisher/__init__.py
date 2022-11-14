import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from esphome.core import CORE
from esphome.components import mylife

DEPENDENCIES = ["spi"]
AUTO_LOAD = ["sensor"]
MULTI_CONF = True

CONF_ENERGY_PUBLISHER = "energy_publisher"
CONF_SENSOR = "sensor"
CONF_MYLIFE_CLIENT = "mylife"

energy_publisher_ns = cg.esphome_ns.namespace("energy_publisher")
ENERGY_PUBLISHER = energy_publisher_ns.class_("EnergyPublisher", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ENERGY_PUBLISHER),
        cv.GenerateID(CONF_MYLIFE_CLIENT): cv.use_id(mylife.MylifeClientComponent),
        cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sensor = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sensor))

    mylife_client = await cg.get_variable(config[CONF_MYLIFE_CLIENT])
    cg.add(var.set_mylife_client(mylife_client))
