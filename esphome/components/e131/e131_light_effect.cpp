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
  call.set_color_mode_if_supported(light::ColorMode::COLD_WARM_WHITE);
  call.set_color_mode_if_supported(light::ColorMode::RGB);
  call.set_color_mode_if_supported(light::ColorMode::RGB_WHITE);
  call.set_color_mode_if_supported(light::ColorMode::RGB_COLOR_TEMPERATURE);
  call.set_color_mode_if_supported(light::ColorMode::RGB_COLD_WARM_WHITE);

  float r = 0.0;
  float g = 0.0;
  float b = 0.0;
  float w = 0.0;
  float cw = 0.0;
  float ww = 0.0;
  float ct = 0.0;

  // set color channels
  // if (channel_map_.red != -1)
  //   r = (float) input_data[channel_map_.red] / 255.0f;
  // if (channel_map_.green != -1)
  //   g = (float) input_data[channel_map_.green] / 255.0f;
  // if (channel_map_.blue != -1)
  //   b = (float) input_data[channel_map_.blue] / 255.0f;
  // // set white channels
  // if (channel_map_.white != -1)
  //   w = (float) input_data[channel_map_.white] / 255.0f;
  // if (channel_map_.cold_white != -1)
  //   cw = (float) input_data[channel_map_.cold_white] / 255.0f;
  // if (channel_map_.warm_white != -1)
  //   ww = (float) input_data[channel_map_.warm_white] / 255.0f;
  // if (channel_map_.color_temp != -1)
  //   ct = (float) input_data[channel_map_.color_temp] / 255.0f;

  if (channel_map_.red != -1) {
    r = (float) input_data[channel_map_.red] / 255.0f;
    call.set_red_if_supported(r);
  }
  if (channel_map_.green != -1) {
    g = (float) input_data[channel_map_.green] / 255.0f;
    call.set_green_if_supported(g);
  }
  if (channel_map_.blue != -1) {
    b = (float) input_data[channel_map_.blue] / 255.0f;
    call.set_blue_if_supported(b);
  }
  // set white channels
  if (channel_map_.white != -1) {
    w = (float) input_data[channel_map_.white] / 255.0f;
    call.set_white_if_supported(w);
  }
  if (channel_map_.cold_white != -1) {
    cw = (float) input_data[channel_map_.cold_white] / 255.0f;
    call.set_cold_white_if_supported(cw);
  }
  if (channel_map_.warm_white != -1) {
    ww = (float) input_data[channel_map_.warm_white] / 255.0f;
    call.set_warm_white_if_supported(ww);
  }
  if (channel_map_.color_temp != -1) {
    ct = (float) input_data[channel_map_.color_temp] / 255.0f;
    call.set_color_temperature_if_supported(ct);
  }

  // call.set_rgbw(r, g, b, w);

  // call.set_red_if_supported(r);
  // call.set_green_if_supported(g);
  // call.set_blue_if_supported(b);
  // call.set_white_if_supported(w);
  // call.set_cold_white_if_supported(cw);
  // call.set_warm_white_if_supported(ww);
  // call.set_color_temperature_if_supported(ct);

  // set brightness to max
  // call.set_brightness_if_supported(std::max(r, std::max(g, std::max(b, std::max(w, std::max(cw, ww))))));
  // call.set_color_brightness_if_supported(r != 0 || g != 0 || b != 0 ? 1.0f : 0.0f);
  call.set_brightness_if_supported(1.0f);
  call.set_color_brightness_if_supported(std::max(r, std::max(g, b)));

  call.set_transition_length_if_supported(0);
  call.set_publish(false);
  call.set_save(false);

  call.perform();

  this->state_->loop();

  return true;
}

}  // namespace e131
}  // namespace esphome

#endif  // USE_ARDUINO
