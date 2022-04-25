import re

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import Condition
from esphome.components import logger
from esphome.components.ota import OTAComponent
from esphome.components.time import RealTimeClock

from esphome.const import (
    CONF_BROKER,
    CONF_CLIENT_ID,
    CONF_ID,
    CONF_KEEPALIVE,
    CONF_OTA,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_REBOOT_TIMEOUT,
    CONF_USERNAME,
)


CONF_TIME = "time"

from esphome.core import coroutine_with_priority


DEPENDENCIES = ["network", "ota", "time", "mqtt"]
AUTO_LOAD = ["json", "async_tcp"]

mylife_ns = cg.esphome_ns.namespace("mylife")
MylifeClientComponent = mylife_ns.class_("MylifeClientComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MylifeClientComponent),
            cv.GenerateID(CONF_OTA): cv.use_id(OTAComponent),
            cv.GenerateID(CONF_TIME): cv.use_id(RealTimeClock),
            cv.Required(CONF_BROKER): cv.string_strict,
            cv.Optional(CONF_PORT, default=1883): cv.port,
            cv.Optional(CONF_USERNAME, default=""): cv.string,
            cv.Optional(CONF_PASSWORD, default=""): cv.string,
            cv.Optional(CONF_CLIENT_ID): cv.string,
            cv.Optional(CONF_KEEPALIVE, default="15s"): cv.positive_time_period_seconds,
            cv.Optional(CONF_REBOOT_TIMEOUT, default="15min"): cv.positive_time_period_milliseconds,
        }
    ),
    cv.only_with_arduino,
)

@coroutine_with_priority(40.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # https://github.com/OttoWinter/async-mqtt-client/blob/master/library.json
    cg.add_library("ottowinter/AsyncMqttClient-esphome", "0.8.6")
    cg.add_define("USE_MYLIFE")

    cg.add(var.set_broker_address(config[CONF_BROKER]))
    cg.add(var.set_broker_port(config[CONF_PORT]))
    cg.add(var.set_username(config[CONF_USERNAME]))
    cg.add(var.set_password(config[CONF_PASSWORD]))
    if CONF_CLIENT_ID in config:
        cg.add(var.set_client_id(config[CONF_CLIENT_ID]))

    cg.add(var.set_keep_alive(config[CONF_KEEPALIVE]))
    cg.add(var.set_reboot_timeout(config[CONF_REBOOT_TIMEOUT]))

    rtc = await cg.get_variable(config[CONF_TIME])
    cg.add(var.set_rtc(rtc))

    ota = await cg.get_variable(config[CONF_OTA])
    cg.add(var.set_ota(ota))

async def mylife_connected_to_code(config, condition_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(condition_id, template_arg, paren)
