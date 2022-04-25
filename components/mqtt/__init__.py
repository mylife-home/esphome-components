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

mqtt_ns = cg.esphome_ns.namespace("mqtt")
MQTTMessage = mqtt_ns.struct("MQTTMessage")
MQTTConfig = mqtt_ns.class_("MQTTConfig", cg.Component)
MQTTComponentStub = mqtt_ns.class_("MQTTComponentStub")

def validate_config(value):
    # Populate default fields
    out = value.copy()
    topic_prefix = value[CONF_TOPIC_PREFIX]
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
            cv.GenerateID(): cv.declare_id(MQTTConfig),
            cv.Required(CONF_BROKER): cv.string_strict,
            cv.Optional(CONF_PORT, default=1883): cv.port,
            cv.Optional(CONF_TOPIC_PREFIX, default=lambda: CORE.name): cv.publish_topic,
            cv.Optional(CONF_DISCOVERY, default=True): cv.Any(
                cv.boolean, cv.one_of("CLEAN", upper=True)
            ),
            cv.Optional(CONF_LOG_TOPIC): cv.Any(
                None,
                MQTT_MESSAGE_BASE.extend(
                    {
                        cv.Optional(CONF_LEVEL): logger.is_log_level,
                    }
                ),
                validate_message_just_topic,
            ),
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

    cg.add_define("USE_MQTT_STUB")
    cg.add_global(mqtt_ns.using)

    cg.add(var.set_topic_prefix(config[CONF_TOPIC_PREFIX]))

    log_topic = config[CONF_LOG_TOPIC]
    if not log_topic:
        cg.add(var.disable_log_message())
    else:
        cg.add(var.set_log_message_template(exp_mqtt_message(log_topic)))

        if CONF_LEVEL in log_topic:
            cg.add(var.set_log_level(logger.LOG_LEVELS[log_topic[CONF_LEVEL]]))

# Re-export MQTT* stuff as normal stuff
MQTTBinarySensorComponent = MQTTComponentStub
MQTTClimateComponent = MQTTComponentStub
MQTTCoverComponent = MQTTComponentStub
MQTTFanComponent = MQTTComponentStub
MQTTJSONLightComponent = MQTTComponentStub
MQTTSensorComponent = MQTTComponentStub
MQTTSwitchComponent = MQTTComponentStub
MQTTTextSensor = MQTTComponentStub
MQTTNumberComponent = MQTTComponentStub
MQTTSelectComponent = MQTTComponentStub
MQTTButtonComponent = MQTTComponentStub
MQTTLockComponent = MQTTComponentStub

async def register_mqtt_component(var, config):
    pass
