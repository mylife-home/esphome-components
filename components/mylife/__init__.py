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
    CONF_CERTIFICATE_AUTHORITY,
    CONF_CLIENT_ID,
    CONF_ID,
    CONF_KEEPALIVE,
    CONF_OTA,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_REBOOT_TIMEOUT,
    CONF_SSL_FINGERPRINTS,
    CONF_USERNAME,
)

from esphome.core import coroutine_with_priority, CORE
from esphome.components.esp32 import add_idf_sdkconfig_option

CONF_TIME = "time"

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
)

@coroutine_with_priority(40.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Add required libraries for arduino
    if CORE.using_arduino:
        # https://github.com/OttoWinter/async-mqtt-client/blob/master/library.json
        cg.add_library("ottowinter/AsyncMqttClient-esphome", "0.8.6")

    cg.add_define("USE_MYLIFE")

    cg.add(var.set_broker_address(config[CONF_BROKER]))
    cg.add(var.set_broker_port(config[CONF_PORT]))
    cg.add(var.set_username(config[CONF_USERNAME]))
    cg.add(var.set_password(config[CONF_PASSWORD]))
    if CONF_CLIENT_ID in config:
        cg.add(var.set_client_id(config[CONF_CLIENT_ID]))

    if CONF_SSL_FINGERPRINTS in config:
        for fingerprint in config[CONF_SSL_FINGERPRINTS]:
            arr = [
                cg.RawExpression(f"0x{fingerprint[i:i + 2]}") for i in range(0, 40, 2)
            ]
            cg.add(var.add_ssl_fingerprint(arr))
        cg.add_build_flag("-DASYNC_TCP_SSL_ENABLED=1")

    cg.add(var.set_keep_alive(config[CONF_KEEPALIVE]))
    cg.add(var.set_reboot_timeout(config[CONF_REBOOT_TIMEOUT]))

    rtc = await cg.get_variable(config[CONF_TIME])
    cg.add(var.set_rtc(rtc))
    ota = await cg.get_variable(config[CONF_OTA])
    cg.add(var.set_ota(ota))

    # esp-idf only
    if CONF_CERTIFICATE_AUTHORITY in config:
        cg.add(var.set_ca_certificate(config[CONF_CERTIFICATE_AUTHORITY]))
        cg.add(var.set_skip_cert_cn_check(config[CONF_SKIP_CERT_CN_CHECK]))

        # prevent error -0x428e
        # See https://github.com/espressif/esp-idf/issues/139
        add_idf_sdkconfig_option("CONFIG_MBEDTLS_HARDWARE_MPI", False)
    # end esp-idf
