import esphome.codegen as cg
from esphome.components import spi, stepper
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.types import ConfigType

DEPENDENCIES = ["spi"]

tmc5160_ns = cg.esphome_ns.namespace("tmc5160")
TMC5160 = tmc5160_ns.class_("TMC5160", stepper.Stepper, cg.Component, spi.SPIDevice)

CONFIG_SCHEMA = (
    stepper.STEPPER_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(TMC5160),
        }
    )
    .extend(spi.spi_device_schema(cs_pin_required=True, default_mode="MODE3"))
    .extend(cv.COMPONENT_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "tmc5160", require_miso=True, require_mosi=True
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await stepper.register_stepper(var, config)
    await spi.register_spi_device(var, config)
