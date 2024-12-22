import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_INDEX
from . import SomfyRtsRemote, somfy_rts_ns

CONF_REMOTE_ID = "remote"
CONF_COMMAND = "command"

DEPENDENCIES = ["somfy_rts"]

SomfyRtsOutput = somfy_rts_ns.class_("SomfyRtsOutput", cg.Component, output.FloatOutput)

Command = somfy_rts_ns.enum("Command", is_class=True)
COMMANDS = {
    "MY": Command.MY,
    "UP": Command.UP,
    "MY_UP": Command.MY_UP,
    "DOWN": Command.DOWN,
    "MY_DOWN": Command.MY_DOWN,
    "UP_DOWN": Command.UP_DOWN,
    "MY_UP_DOWN": Command.MY_UP_DOWN,
    "PROG": Command.PROG,
    "SUN_FLAG": Command.SUN_FLAG,
    "FLAG": Command.FLAG,
}

CONFIG_SCHEMA = output.BINARY_OUTPUT_SCHEMA.extend(cv.COMPONENT_SCHEMA).extend(
    {
        cv.Required(CONF_ID): cv.declare_id(SomfyRtsOutput),
        cv.GenerateID(CONF_REMOTE_ID): cv.use_id(SomfyRtsRemote),
        cv.Required(CONF_COMMAND): cv.enum(COMMANDS, upper=True, space="_"),
    }
)

async def to_code(config):
    command = config[CONF_COMMAND]
    remote = await cg.get_variable(config[CONF_REMOTE_ID])

    var = cg.new_Pvariable(config[CONF_ID], command, remote)
    await cg.register_component(var, config)
    await output.register_output(var, config)
