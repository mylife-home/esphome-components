import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import remote_base
from esphome.const import (
    CONF_ID,
    CONF_PIN,
    CONF_ADDRESS
)

MULTI_CONF = True
CONF_REAPEAT = "repeat"

DEPENDENCIES = ["remote_transmitter"]
AUTO_LOAD = ["sensor", "remote_base"]

somfy_rts_ns = cg.esphome_ns.namespace("somfy_rts")

SomfyRtsRemote = somfy_rts_ns.class_("SomfyRtsRemote", cg.Component, remote_base.RemoteTransmittable)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(SomfyRtsRemote),
            cv.Required(CONF_ADDRESS): cv.All(cv.hex_int, cv.Range(min=0, max=0xFFFFFF)),
            cv.Optional(CONF_REAPEAT, default=1): cv.positive_int,
        }
    )
    .extend(remote_base.REMOTE_TRANSMITTABLE_SCHEMA)
)


async def to_code(config):
    address = config[CONF_ADDRESS]
    repeat = config[CONF_REAPEAT]

    var = cg.new_Pvariable(config[CONF_ID], address, repeat)
    await cg.register_component(var, config)
    await remote_base.register_transmittable(var, config)
