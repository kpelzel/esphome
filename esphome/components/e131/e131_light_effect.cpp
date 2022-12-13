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

// int E131LightEffect::get_data_per_universe() const { return get_lights_per_universe() * channels_; }

// int E131LightEffect::get_lights_per_universe() const { return MAX_DATA_SIZE / channels_; }

// int E131LightEffect::get_first_universe() const { return first_universe_; }

// int E131LightEffect::get_last_universe() const { return first_universe_ + get_universe_count() - 1; }

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
  ESP_LOGD(TAG, "BLAH");
  ESP_LOGV(TAG, "Applying data for '%s' on %d universe, for .", get_name().c_str(), universe);

  // check if this is our universe and data are valid
  if (universe < first_universe_ || universe > get_last_universe()) {
    ESP_LOGV(TAG, "wrong universe %d %d %d", universe, first_universe_, get_last_universe());
    return false;
  }

  int output_offset = (universe - first_universe_) * get_lights_per_universe();
  // limit amount of lights per universe and received
  // int output_end =
  //     std::min(it->size(), std::min(output_offset + get_lights_per_universe(), output_offset + packet.count - 1));
  auto *input_data = packet.values + 1;

  ESP_LOGV(TAG, "Applying data for '%s' on %d universe, for %d-%d.", get_name().c_str(), universe, output_offset,
           output_offset);

  auto call = this->state_->turn_on();

  switch (channels_) {
    case E131_MONO:
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLD_WARM_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLOR_TEMPERATURE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB);
      call.set_red_if_supported(input_data[0]);
      call.set_green_if_supported(input_data[0]);
      call.set_blue_if_supported(input_data[0]);
      call.set_brightness_if_supported(std::max(input_data[0], std::max(input_data[0], input_data[0])));
      call.set_color_brightness_if_supported(1.0f);

      // disable white channels
      call.set_white_if_supported(0.0f);
      call.set_cold_white_if_supported(0.0f);
      call.set_warm_white_if_supported(0.0f);

      break;
    case E131_RGB:
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLD_WARM_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLOR_TEMPERATURE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB);
      call.set_red_if_supported(input_data[0]);
      call.set_green_if_supported(input_data[1]);
      call.set_blue_if_supported(input_data[2]);
      call.set_brightness_if_supported(std::max(input_data[0], std::max(input_data[1], input_data[2])));
      call.set_color_brightness_if_supported(1.0f);

      // disable white channels
      call.set_white_if_supported(0.0f);
      call.set_cold_white_if_supported(0.0f);
      call.set_warm_white_if_supported(0.0f);

      break;
    case E131_RGBW:
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLD_WARM_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_COLOR_TEMPERATURE);
      call.set_color_mode_if_supported(light::ColorMode::RGB_WHITE);
      call.set_color_mode_if_supported(light::ColorMode::RGB);
      call.set_red_if_supported(input_data[0]);
      call.set_green_if_supported(input_data[1]);
      call.set_blue_if_supported(input_data[2]);
      call.set_brightness_if_supported(std::max(input_data[0], std::max(input_data[1], input_data[2])));
      call.set_color_brightness_if_supported(1.0f);

      // disable white channels
      call.set_white_if_supported(input_data[3]);
      call.set_cold_white_if_supported(input_data[3]);
      call.set_warm_white_if_supported(input_data[3]);
  }

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
