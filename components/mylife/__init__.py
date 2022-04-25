import re

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import Condition
from esphome.components import logger
from esphome.const import (
    CONF_AVAILABILITY,
    CONF_BIRTH_MESSAGE,
    CONF_BROKER,
    CONF_CERTIFICATE_AUTHORITY,
    CONF_CLIENT_ID,
    CONF_COMMAND_TOPIC,
    CONF_COMMAND_RETAIN,
    CONF_DISCOVERY,
    CONF_DISCOVERY_PREFIX,
    CONF_DISCOVERY_RETAIN,
    CONF_DISCOVERY_UNIQUE_ID_GENERATOR,
    CONF_DISCOVERY_OBJECT_ID_GENERATOR,
    CONF_ID,
    CONF_KEEPALIVE,
    CONF_LEVEL,
    CONF_LOG_TOPIC,
    CONF_ON_JSON_MESSAGE,
    CONF_ON_MESSAGE,
    CONF_PASSWORD,
    CONF_PAYLOAD,
    CONF_PAYLOAD_AVAILABLE,
    CONF_PAYLOAD_NOT_AVAILABLE,
    CONF_PORT,
    CONF_QOS,
    CONF_REBOOT_TIMEOUT,
    CONF_RETAIN,
    CONF_SHUTDOWN_MESSAGE,
    CONF_SSL_FINGERPRINTS,
    CONF_STATE_TOPIC,
    CONF_TOPIC,
    CONF_TOPIC_PREFIX,
    CONF_TRIGGER_ID,
    CONF_USE_ABBREVIATIONS,
    CONF_USERNAME,
    CONF_WILL_MESSAGE,
)
from esphome.core import coroutine_with_priority, CORE
from esphome.components.esp32 import add_idf_sdkconfig_option

DEPENDENCIES = ["network"]

AUTO_LOAD = ["json"]

def validate_message_just_topic(value):
    value = cv.publish_topic(value)
    return MQTT_MESSAGE_BASE({CONF_TOPIC: value})


MQTT_MESSAGE_BASE = cv.Schema(
    {
        cv.Required(CONF_TOPIC): cv.publish_topic,
        cv.Optional(CONF_QOS, default=0): cv.mqtt_qos,
        cv.Optional(CONF_RETAIN, default=True): cv.boolean,
    }
)

MQTT_MESSAGE_TEMPLATE_SCHEMA = cv.Any(
    None, MQTT_MESSAGE_BASE, validate_message_just_topic
)

MQTT_MESSAGE_SCHEMA = cv.Any(
    None,
    MQTT_MESSAGE_BASE.extend(
        {
            cv.Required(CONF_PAYLOAD): cv.mqtt_payload,
        }
    ),
)

mylife_ns = cg.esphome_ns.namespace("mylife")
mqtt_ns = cg.esphome_ns.namespace("mqtt")
MQTTMessage = mqtt_ns.struct("MQTTMessage")
MQTTClientComponent = mylife_ns.class_("MQTTClientComponent", cg.Component)

MQTTDiscoveryUniqueIdGenerator = mylife_ns.enum("MQTTDiscoveryUniqueIdGenerator")
MQTT_DISCOVERY_UNIQUE_ID_GENERATOR_OPTIONS = {
    "legacy": MQTTDiscoveryUniqueIdGenerator.MQTT_LEGACY_UNIQUE_ID_GENERATOR,
    "mac": MQTTDiscoveryUniqueIdGenerator.MQTT_MAC_ADDRESS_UNIQUE_ID_GENERATOR,
}

MQTTDiscoveryObjectIdGenerator = mylife_ns.enum("MQTTDiscoveryObjectIdGenerator")
MQTT_DISCOVERY_OBJECT_ID_GENERATOR_OPTIONS = {
    "none": MQTTDiscoveryObjectIdGenerator.MQTT_NONE_OBJECT_ID_GENERATOR,
    "device_name": MQTTDiscoveryObjectIdGenerator.MQTT_DEVICE_NAME_OBJECT_ID_GENERATOR,
}


def validate_config(value):
    # Populate default fields
    out = value.copy()
    topic_prefix = value[CONF_TOPIC_PREFIX]
    if CONF_BIRTH_MESSAGE not in value:
        out[CONF_BIRTH_MESSAGE] = {
            CONF_TOPIC: f"{topic_prefix}/status",
            CONF_PAYLOAD: "online",
            CONF_QOS: 0,
            CONF_RETAIN: True,
        }
    if CONF_WILL_MESSAGE not in value:
        out[CONF_WILL_MESSAGE] = {
            CONF_TOPIC: f"{topic_prefix}/status",
            CONF_PAYLOAD: "offline",
            CONF_QOS: 0,
            CONF_RETAIN: True,
        }
    if CONF_SHUTDOWN_MESSAGE not in value:
        out[CONF_SHUTDOWN_MESSAGE] = {
            CONF_TOPIC: f"{topic_prefix}/status",
            CONF_PAYLOAD: "offline",
            CONF_QOS: 0,
            CONF_RETAIN: True,
        }
    if CONF_LOG_TOPIC not in value:
        out[CONF_LOG_TOPIC] = {
            CONF_TOPIC: f"{topic_prefix}/debug",
            CONF_QOS: 0,
            CONF_RETAIN: True,
        }
    return out


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MQTTClientComponent),
            cv.Required(CONF_BROKER): cv.string_strict,
            cv.Optional(CONF_PORT, default=1883): cv.port,
            cv.Optional(CONF_USERNAME, default=""): cv.string,
            cv.Optional(CONF_PASSWORD, default=""): cv.string,
            cv.Optional(CONF_CLIENT_ID): cv.string,
            cv.Optional(CONF_DISCOVERY, default=True): cv.Any(
                cv.boolean, cv.one_of("CLEAN", upper=True)
            ),
            cv.Optional(CONF_DISCOVERY_RETAIN, default=True): cv.boolean,
            cv.Optional(
                CONF_DISCOVERY_PREFIX, default="homeassistant"
            ): cv.publish_topic,
            cv.Optional(CONF_DISCOVERY_UNIQUE_ID_GENERATOR, default="legacy"): cv.enum(
                MQTT_DISCOVERY_UNIQUE_ID_GENERATOR_OPTIONS
            ),
            cv.Optional(CONF_DISCOVERY_OBJECT_ID_GENERATOR, default="none"): cv.enum(
                MQTT_DISCOVERY_OBJECT_ID_GENERATOR_OPTIONS
            ),
            cv.Optional(CONF_USE_ABBREVIATIONS, default=True): cv.boolean,
            cv.Optional(CONF_BIRTH_MESSAGE): MQTT_MESSAGE_SCHEMA,
            cv.Optional(CONF_WILL_MESSAGE): MQTT_MESSAGE_SCHEMA,
            cv.Optional(CONF_SHUTDOWN_MESSAGE): MQTT_MESSAGE_SCHEMA,
            cv.Optional(CONF_TOPIC_PREFIX, default=lambda: CORE.name): cv.publish_topic,
            cv.Optional(CONF_LOG_TOPIC): cv.Any(
                None,
                MQTT_MESSAGE_BASE.extend(
                    {
                        cv.Optional(CONF_LEVEL): logger.is_log_level,
                    }
                ),
                validate_message_just_topic,
            ),
            cv.Optional(CONF_KEEPALIVE, default="15s"): cv.positive_time_period_seconds,
            cv.Optional(
                CONF_REBOOT_TIMEOUT, default="15min"
            ): cv.positive_time_period_milliseconds,
        }
    ),
    validate_config,
)


def exp_mqtt_message(config):
    if config is None:
        return cg.optional(cg.TemplateArguments(MQTTMessage))
    exp = cg.StructInitializer(
        MQTTMessage,
        ("topic", config[CONF_TOPIC]),
        ("payload", config.get(CONF_PAYLOAD, "")),
        ("qos", config[CONF_QOS]),
        ("retain", config[CONF_RETAIN]),
    )
    return exp


@coroutine_with_priority(40.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Add required libraries for arduino
    if CORE.using_arduino:
        # https://github.com/OttoWinter/async-mqtt-client/blob/master/library.json
        cg.add_library("ottowinter/AsyncMqttClient-esphome", "0.8.6")

    cg.add_define("USE_MYLIFE")
    cg.add_global(mqtt_ns.using)

    cg.add(var.set_broker_address(config[CONF_BROKER]))
    cg.add(var.set_broker_port(config[CONF_PORT]))
    cg.add(var.set_username(config[CONF_USERNAME]))
    cg.add(var.set_password(config[CONF_PASSWORD]))
    if CONF_CLIENT_ID in config:
        cg.add(var.set_client_id(config[CONF_CLIENT_ID]))

    cg.add(var.set_topic_prefix(config[CONF_TOPIC_PREFIX]))

    if config[CONF_USE_ABBREVIATIONS]:
        cg.add_define("USE_MQTT_ABBREVIATIONS")

    birth_message = config[CONF_BIRTH_MESSAGE]
    if not birth_message:
        cg.add(var.disable_birth_message())
    else:
        cg.add(var.set_birth_message(exp_mqtt_message(birth_message)))
    will_message = config[CONF_WILL_MESSAGE]
    if not will_message:
        cg.add(var.disable_last_will())
    else:
        cg.add(var.set_last_will(exp_mqtt_message(will_message)))
    shutdown_message = config[CONF_SHUTDOWN_MESSAGE]
    if not shutdown_message:
        cg.add(var.disable_shutdown_message())
    else:
        cg.add(var.set_shutdown_message(exp_mqtt_message(shutdown_message)))

    log_topic = config[CONF_LOG_TOPIC]
    if not log_topic:
        cg.add(var.disable_log_message())
    else:
        cg.add(var.set_log_message_template(exp_mqtt_message(log_topic)))

        if CONF_LEVEL in log_topic:
            cg.add(var.set_log_level(logger.LOG_LEVELS[log_topic[CONF_LEVEL]]))

    if CONF_SSL_FINGERPRINTS in config:
        for fingerprint in config[CONF_SSL_FINGERPRINTS]:
            arr = [
                cg.RawExpression(f"0x{fingerprint[i:i + 2]}") for i in range(0, 40, 2)
            ]
            cg.add(var.add_ssl_fingerprint(arr))
        cg.add_build_flag("-DASYNC_TCP_SSL_ENABLED=1")

    cg.add(var.set_keep_alive(config[CONF_KEEPALIVE]))

    cg.add(var.set_reboot_timeout(config[CONF_REBOOT_TIMEOUT]))

    # esp-idf only
    if CONF_CERTIFICATE_AUTHORITY in config:
        cg.add(var.set_ca_certificate(config[CONF_CERTIFICATE_AUTHORITY]))
        cg.add(var.set_skip_cert_cn_check(config[CONF_SKIP_CERT_CN_CHECK]))

        # prevent error -0x428e
        # See https://github.com/espressif/esp-idf/issues/139
        add_idf_sdkconfig_option("CONFIG_MBEDTLS_HARDWARE_MPI", False)
    # end esp-idf

def get_default_topic_for(data, component_type, name, suffix):
    allowlist = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_"
    sanitized_name = "".join(
        x for x in name.lower().replace(" ", "_") if x in allowlist
    )
    return f"{data.topic_prefix}/{component_type}/{sanitized_name}/{suffix}"
