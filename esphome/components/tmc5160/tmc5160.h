#pragma once

#include "esphome/components/spi/spi.h"
#include "esphome/components/stepper/stepper.h"
#include "esphome/core/component.h"

namespace esphome::tmc5160 {

/// Stepper driver for the TMC5160 in SPI mode, using the chip's internal
/// ramp generator (RAMPMODE/XTARGET) instead of an external STEP/DIR clock.
/// Positions and speeds are in full steps; the driver converts to the chip's
/// native 1/256 microstep units.
class TMC5160 : public Component,
                public stepper::Stepper,
                public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_TRAILING,
                                      spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void stop_motion();
  void set_motor_enabled(bool enabled);
  bool motor_enabled_{true};

  void on_update_speed() override { this->apply_motion_config_(); }

 protected:
  enum Register : uint8_t {
    REGISTER_GCONF = 0x00,
    REGISTER_GSTAT = 0x01,
    REGISTER_IHOLD_IRUN = 0x10,
    REGISTER_RAMPMODE = 0x20,
    REGISTER_XACTUAL = 0x21,
    REGISTER_VSTART = 0x23,
    REGISTER_A1 = 0x24,
    REGISTER_V1 = 0x25,
    REGISTER_AMAX = 0x26,
    REGISTER_VMAX = 0x27,
    REGISTER_DMAX = 0x28,
    REGISTER_D1 = 0x2A,
    REGISTER_VSTOP = 0x2B,
    REGISTER_XTARGET = 0x2D,
    REGISTER_CHOPCONF = 0x6C,
    REGISTER_DRV_STATUS = 0x6F,
  };

  uint32_t read_reg_(uint8_t address, bool log_raw = false);
  void write_reg_(uint8_t address, uint32_t value);
  void apply_motion_config_();

  /// Last target written to XTARGET, used to detect config changes in loop().
  int32_t written_target_{0};
  uint32_t configured_vmax_{800000};
};

}  // namespace esphome::tmc5160
