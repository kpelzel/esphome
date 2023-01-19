import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.types import AddressableLightEffect, LightEffect
from esphome.components.light.effects import (
    register_addressable_effect,
    register_rgb_effect,
)
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_METHOD,
    CONF_CHANNELS,
    CONF_RED,
    CONF_GREEN,
    CONF_BLUE,
    CONF_WHITE,
    CONF_WARM_WHITE,
    CONF_COLD_WHITE,
    CONF_COLOR_TEMPERATURE,
)

CONF_CHANNEL_MAP = "channel_map"

AUTO_LOAD = ["socket"]
DEPENDENCIES = ["network"]

e131_ns = cg.esphome_ns.namespace("e131")
E131AddressableLightEffect = e131_ns.class_(
    "E131AddressableLightEffect", AddressableLightEffect
)
E131LightEffect = e131_ns.class_("E131LightEffect", LightEffect)
E131Component = e131_ns.class_("E131Component", cg.Component)
E131ChannelMap = e131_ns.class_("E131ChannelMap", LightEffect)


METHODS = {"UNICAST": e131_ns.E131_UNICAST, "MULTICAST": e131_ns.E131_MULTICAST}

CHANNELS = {
    "MONO": e131_ns.E131_MONO,
    "RGB": e131_ns.E131_RGB,
    "RGBW": e131_ns.E131_RGBW,
}

CONF_UNIVERSE = "universe"
CONF_E131_ID = "e131_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(E131Component),
        cv.Optional(CONF_METHOD, default="MULTICAST"): cv.one_of(*METHODS, upper=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_method(METHODS[config[CONF_METHOD]]))


@register_rgb_effect(
    "non_addressable_e131",
    E131LightEffect,
    "Non-Addressable E1.31",
    {
        cv.GenerateID(CONF_E131_ID): cv.use_id(E131Component),
        cv.Required(CONF_UNIVERSE): cv.int_range(min=1, max=512),
        cv.Optional(
            CONF_CHANNEL_MAP, default={CONF_RED: 0, CONF_GREEN: 1, CONF_BLUE: 2}
        ): cv.All(
            cv.Schema(
                {
                    cv.Optional(CONF_RED): cv.int_range(0, 512),
                    cv.Optional(CONF_GREEN): cv.int_range(0, 512),
                    cv.Optional(CONF_BLUE): cv.int_range(0, 512),
                    cv.Optional(CONF_WHITE): cv.int_range(0, 512),
                    cv.Optional(CONF_WARM_WHITE): cv.int_range(0, 512),
                    cv.Optional(CONF_COLD_WHITE): cv.int_range(0, 512),
                    cv.Optional(CONF_COLOR_TEMPERATURE): cv.int_range(0, 512),
                }
            ),
            cv.has_at_least_one_key(
                CONF_RED,
                CONF_GREEN,
                CONF_BLUE,
                CONF_WHITE,
                CONF_WARM_WHITE,
                CONF_COLD_WHITE,
            ),
        ),
    },
)
@register_addressable_effect(
    "e131",
    E131AddressableLightEffect,
    "E1.31",
    {
        cv.GenerateID(CONF_E131_ID): cv.use_id(E131Component),
        cv.Required(CONF_UNIVERSE): cv.int_range(min=1, max=512),
        cv.Optional(CONF_CHANNELS, default="RGB"): cv.one_of(*CHANNELS, upper=True),
    },
)
async def e131_light_effect_to_code(config, effect_id):
    parent = await cg.get_variable(config[CONF_E131_ID])

    effect = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(effect.set_first_universe(config[CONF_UNIVERSE]))
    cg.add(effect.set_e131(parent))

    if CONF_CHANNEL_MAP in config:
        conf_channel_map = config.get(CONF_CHANNEL_MAP)
        channel_map = cg.StructInitializer(
            E131ChannelMap,
            (
                "red",
                conf_channel_map.get(CONF_RED, -1),
            ),
            (
                "green",
                conf_channel_map.get(CONF_GREEN, -1),
            ),
            (
                "blue",
                conf_channel_map.get(CONF_BLUE, -1),
            ),
            (
                "white",
                conf_channel_map.get(CONF_WHITE, -1),
            ),
            (
                "warm_white",
                conf_channel_map.get(CONF_WARM_WHITE, -1),
            ),
            (
                "cold_white",
                conf_channel_map.get(CONF_COLD_WHITE, -1),
            ),
            (
                "color_temp",
                conf_channel_map.get(CONF_COLOR_TEMPERATURE, -1),
            ),
        )
        cg.add(effect.set_channel_map(channel_map))

    if CONF_CHANNELS in config:
        cg.add(effect.set_channels(CHANNELS[config[CONF_CHANNELS]]))

    return effect
