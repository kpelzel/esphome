#ifdef USE_ARDUINO

#include "e131.h"
#include "e131_light_effect.h"
#include "esphome/core/log.h"

namespace esphome {
namespace e131 {

static const char *const TAG = "e131_light_effect";
// static const int MAX_DATA_SIZE = (sizeof(E131Packet::values) - 1);

E131LightEffect::E131LightEffect(const std::string &name) : LightEffect(name) {}

const std::string &E131LightEffect::get_name() { return LightEffect::get_name(); }

int E131LightEffect::get_universe_count() const {
  // Round up to lights_per_universe
  auto lights = get_lights_per_universe();
  return (1 + lights - 1) / lights;
}

void E131LightEffect::start() {
  LightEffect::start();
  E131LightEffectBase::start();
}

void E131LightEffect::stop() {
  E131LightEffectBase::stop();
  LightEffect::stop();
}

void E131LightEffect::apply() {
  // ignore, it is run by `E131Component::update()`
}

bool E131LightEffect::process_(int universe, const E131Packet &packet) {
  // check if this is our universe and data are valid
  if (universe < first_universe_ || universe > get_last_universe()) {
    ESP_LOGV(TAG, "received packet with wrong universe. Looking for universe between %d and %d, got %d",
             first_universe_, get_last_universe(), universe);
    return false;
  }

  auto *input_data = packet.values + 1;

  auto call = this->state_->turn_on();

  // set color mode
  call.set_color_mode_if_supported(light::ColorMode::WHITE);
  call.set_color_mode_if_supported(light::ColorMode::COLOR_TEMPERATURE);
  call.set_color_mode_if_supported(light::ColorMode::WARM_WHITE);
  call.set_color_mode_if_supported(light::ColorMode::RGB);
  call.set_color_mode_if_supported(light::ColorMode::RGB_WHITE);
  call.set_color_mode_if_supported(light::ColorMode::RGB_COLOR_TEMPERATURE);
  call.set_color_mode_if_supported(light::ColorMode::RGB_COLD_WARM_WHITE);

  // set color channels
  if (channel_map_.red != -1) {
    float r = (float) input_data[channel_map_.red] / 255.0f;
    call.set_red_if_supported(r);
  }
  if (channel_map_.green != -1) {
    float g = (float) input_data[channel_map_.green] / 255.0f;
    call.set_green_if_supported(g);
  }
  if (channel_map_.blue != -1) {
    float b = (float) input_data[channel_map_.blue] / 255.0f;
    call.set_blue_if_supported(b);
  }
  // set white channels
  if (channel_map_.white != -1) {
    float w = (float) input_data[channel_map_.white] / 255.0f;
    call.set_white_if_supported(w);
  }
  if (channel_map_.cold_white != -1) {
    float cw = (float) input_data[channel_map_.cold_white] / 255.0f;
    call.set_cold_white_if_supported(cw);
  }
  if (channel_map_.warm_white != -1) {
    float ww = (float) input_data[channel_map_.warm_white] / 255.0f;
    call.set_warm_white_if_supported(ww);
  }
  if (channel_map_.color_temp != -1) {
    float ct = (float) input_data[channel_map_.color_temp] / 255.0f;
    call.set_color_temperature_if_supported(ct);
  }

  // set brightness to max
  call.set_brightness_if_supported(1.0f);
  call.set_color_brightness_if_supported(1.0f);

  call.set_transition_length_if_supported(0);
  call.set_publish(false);
  call.set_save(false);

  call.perform();

  // manually calling loop otherwise we just go straight into processing the next DDP
  // packet without executing the light loop to display the just-processed packet.
  // Not totally sure why or if there is a better way to fix, but this works.
  this->state_->loop();

  return true;
}

}  // namespace e131
}  // namespace esphome

#endif  // USE_ARDUINO
