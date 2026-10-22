#include "tmc5160.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <array>
#include <cinttypes>

namespace esphome::tmc5160 {

ESPHOME_LOG_TAG(TAG, "tmc5160");

// The TMC5160 ramps in units of 1/256 microsteps and internal clock ticks.
// Register value = full steps (or full steps/s, steps/s^2) times these factors,
// with the chip's native 1/256 microstep units and an internal clock of 12 MHz:
//   velocity: 2^24 * 256 / 12e6, acceleration: 2^41 * 256 / 12e6^2
static constexpr double VELOCITY_REG_PER_STEP_PER_S = 357.91394;
static constexpr double ACCEL_REG_PER_STEP_PER_S2 = 3.90937;

// Largest value the chip accepts for the 23-bit VMAX, and the 16-bit A1/AMAX/DMAX fields.
static constexpr uint32_t MAX_VELOCITY = (1U << 23) - 1U;
static constexpr uint32_t MAX_ACCEL = (1U << 16) - 1U;

// Smallest threshold velocity for the final ramp phase (native units).
static constexpr uint32_t VSTOP_NATIVE = 10;

// Position mode, full run current, no standstill reduction.
static constexpr uint32_t RAMPMODE_POSITION = 0x00;
static constexpr uint32_t IHOLD_IRUN_FULL = 0x00081F1F;

void TMC5160::write_reg_(uint8_t address, uint32_t value) {
  std::array<uint8_t, 5> data{
      static_cast<uint8_t>(address | 0x80), static_cast<uint8_t>(value >> 24), static_cast<uint8_t>(value >> 16),
      static_cast<uint8_t>(value >> 8),     static_cast<uint8_t>(value),
  };

  ESP_LOGD(TAG, "WRITE reg 0x%02X: %02X %02X %02X %02X %02X", address, data[0], data[1], data[2], data[3], data[4]);

  this->enable();
  this->transfer_array(data.data(), data.size());
  this->disable();
}

uint32_t TMC5160::read_reg_(uint8_t address, bool log_raw) {
  std::array<uint8_t, 5> request{static_cast<uint8_t>(address & 0x7F), 0, 0, 0, 0};

  std::array<uint8_t, 5> response{};

  this->enable();
  this->transfer_array(request.data(), request.size());
  this->disable();

  this->enable();
  this->transfer_array(response.data(), response.size());
  this->disable();

  if (log_raw) {
    ESP_LOGD(TAG, "READ reg 0x%02X response: %02X %02X %02X %02X %02X", address, response[0], response[1], response[2],
             response[3], response[4]);
  }

  return (uint32_t(response[1]) << 24) | (uint32_t(response[2]) << 16) | (uint32_t(response[3]) << 8) |
         uint32_t(response[4]);
}

void TMC5160::apply_motion_config_() {
  const uint32_t vmax =
      static_cast<uint32_t>(std::min<float>(this->max_speed_ * VELOCITY_REG_PER_STEP_PER_S, MAX_VELOCITY));
  const uint32_t amax =
      static_cast<uint32_t>(std::min<float>(this->acceleration_ * ACCEL_REG_PER_STEP_PER_S2, MAX_ACCEL));
  const uint32_t dmax =
      static_cast<uint32_t>(std::min<float>(this->deceleration_ * ACCEL_REG_PER_STEP_PER_S2, MAX_ACCEL));
  this->write_reg_(REGISTER_VSTART, 0);
  this->write_reg_(REGISTER_V1, 0);
  this->write_reg_(REGISTER_A1, amax);
  this->write_reg_(REGISTER_AMAX, amax);
  this->write_reg_(REGISTER_VMAX, vmax);
  this->write_reg_(REGISTER_DMAX, dmax);
  this->write_reg_(REGISTER_D1, dmax);
  this->write_reg_(REGISTER_VSTOP, VSTOP_NATIVE);
}

void TMC5160::setup() {
  this->spi_setup();

  ESP_LOGI(TAG, "Initializing TMC5160 using datasheet ramp-generator example");

  // Clear latched status flags.
  this->write_reg_(REGISTER_GSTAT, 0x00000007);

  // Configure SpreadCycle chopper.
  // CHOPCONF (0x6C): TOFF=3, HSTRT=4, HEND=1, TBL=2.
  this->write_reg_(0x6C, 0x000100C3);

  // Current configuration from the datasheet example.
  // WARNING: IRUN=31 is maximum scale; verify motor current and
  // the breakout's sense-resistor configuration before using this.
  // this->write_reg_(REGISTER_IHOLD_IRUN, 0x00060F07);

  // Set current scale to approximately 1.4 A RMS.
  this->write_reg_(0x0B, 115);  // GLOBALSCALER

  // // IHOLDDELAY=6, IRUN=31, IHOLD=19.
  // this->write_reg_(REGISTER_IHOLD_IRUN, 0x00061F13);

  // // IHOLDDELAY=6, IRUN=31, IHOLD=10
  // this->write_reg_(REGISTER_IHOLD_IRUN, 0x00060A1F);

  // IHOLDDELAY=6, IRUN=31, IHOLD=5
  this->write_reg_(REGISTER_IHOLD_IRUN, 0x0006051F);

  // TPOWERDOWN = 10.
  this->write_reg_(0x11, 0x0000000A);

  // Enable StealthChop mode (EN_PWM_MODE=1).
  this->write_reg_(REGISTER_GCONF, 0x00000004);

  // TPWM_THRS = 500.
  this->write_reg_(0x13, 0x000001F4);

  // Initialize the internal ramp generator.
  // this->write_reg_(REGISTER_A1, 1000);
  // this->write_reg_(REGISTER_V1, 200000);  // 2x
  // this->write_reg_(REGISTER_AMAX, 500);
  // this->write_reg_(REGISTER_VMAX, 800000);  // 2x
  // this->write_reg_(REGISTER_DMAX, 1100);
  // this->write_reg_(REGISTER_D1, 2000);
  // this->write_reg_(REGISTER_VSTOP, 10);

  // Initialize the internal ramp generator.
  this->write_reg_(REGISTER_A1, 5000);
  this->write_reg_(REGISTER_V1, 200000);  // 2x
  this->write_reg_(REGISTER_AMAX, 2500);
  this->write_reg_(REGISTER_VMAX, 800000);  // 2x
  this->write_reg_(REGISTER_DMAX, 2500);
  this->write_reg_(REGISTER_D1, 5000);
  this->write_reg_(REGISTER_VSTOP, 10);

  // Positioning mode.
  this->write_reg_(REGISTER_RAMPMODE, 0);

  // Datasheet test: move to -51200 microsteps.
  // -51200 as a signed 32-bit value is 0xFFFF3800.
  this->write_reg_(REGISTER_XTARGET, 0xFFFF3800);

  ESP_LOGI(TAG, "Sent XTARGET=-51200; checking ramp-generator registers");

  ESP_LOGI(TAG,
           "GCONF=%08" PRIX32 " CHOPCONF=%08" PRIX32 " RAMPMODE=%08" PRIX32 " VMAX=%08" PRIX32 " XTARGET=%08" PRIX32,
           this->read_reg_(REGISTER_GCONF), this->read_reg_(0x6C), this->read_reg_(REGISTER_RAMPMODE),
           this->read_reg_(REGISTER_VMAX), this->read_reg_(REGISTER_XTARGET));

  // Keep ESPHome's software position bookkeeping separate from this
  // raw-register test. Restore normal Stepper integration after testing.
}

void TMC5160::loop() {
  if (this->target_position != this->written_target_) {
    this->written_target_ = this->target_position;

    // Restore the configured speed before starting the next move.
    this->write_reg_(REGISTER_VSTART, 0);
    this->write_reg_(REGISTER_VMAX, this->configured_vmax_);

    ESP_LOGD(TAG, "Target changed to %" PRId32 ", writing XTARGET", this->written_target_);

    this->write_reg_(REGISTER_XTARGET, static_cast<uint32_t>(static_cast<int32_t>(this->written_target_)) * 256);
  }

  const int32_t actual = static_cast<int32_t>(this->read_reg_(REGISTER_XACTUAL)) >> 8;

  if (actual != this->current_position) {
    this->current_position = actual;
  }
}

void TMC5160::stop_motion() {
  // Snapshot the current hardware position for ESPHome bookkeeping.
  const int32_t actual_raw = static_cast<int32_t>(this->read_reg_(REGISTER_XACTUAL));
  const int32_t actual_steps = actual_raw >> 8;

  // Prevent loop() from rewriting the old target.
  this->current_position = actual_steps;
  this->target_position = actual_steps;
  this->written_target_ = actual_steps;

  // TMC5160 datasheet: early ramp termination in positioning mode.
  // Stop using AMAX/A1 rather than correcting toward the old target.
  this->write_reg_(REGISTER_VSTART, 0);
  this->write_reg_(REGISTER_VMAX, 0);

  ESP_LOGI(TAG, "Controlled stop requested at position %" PRId32, actual_steps);
}

void TMC5160::dump_config() {
  ESP_LOGCONFIG(TAG, "TMC5160:");
  LOG_STEPPER(this);
}

}  // namespace esphome::tmc5160
