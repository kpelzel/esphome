#pragma once

#ifdef USE_ARDUINO

#include "esphome/core/component.h"
#include "esphome/components/light/light_effect.h"
#include "esphome/components/light/light_output.h"
#include "e131_light_effect_base.h"

namespace esphome {
namespace e131 {

// class E131Component;
// struct E131Packet;

// enum E131LightChannels { E131_MONO = 1, E131_RGB = 3, E131_RGBW = 4 };

class E131LightEffect : public E131LightEffectBase, public light::LightEffect {
 public:
  E131LightEffect(const std::string &name);

  const std::string &get_name() override;

  void start() override;
  void stop() override;
  void apply() override;

  int get_data_per_universe() const override;
  int get_lights_per_universe() const override;
  int get_first_universe() const override;
  int get_last_universe() const override;
  int get_universe_count() const override;

 protected:
  bool process_(int universe, const E131Packet &packet) override;

  // int first_universe_{0};
  // int last_universe_{0};
  // E131LightChannels channels_{E131_RGB};
  // E131Component *e131_{nullptr};

  // friend class E131Component;
};

}  // namespace e131
}  // namespace esphome

#endif  // USE_ARDUINO
